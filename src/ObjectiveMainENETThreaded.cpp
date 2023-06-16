/*
 *  ObjectiveMainENETThreaded.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "ObjectiveMainENETThreaded.h"
#include "GSUtil.h"
#include "DataFile.h"
#include "Simulation.h"
#include "Reporter.h"
#include "DataTarget.h"
#include "Driver.h"
#include "Joint.h"
#include "Muscle.h"
#include "Body.h"
#include "Geom.h"
#include "ArgParse.h"
#include "MD5.h"

#include "pystring.h"

#include <chrono>
#include <thread>
#include <algorithm>
#include <random>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <WinSock2.h>
#else
#include <netdb.h>
#endif

#define MAX_ARGS 4096

using namespace std::string_literals;

#if defined(USE_ENET_THREADED)
int main(int argc, const char **argv)
{
    ObjectiveMainENETThreaded objectiveMain(argc, argv);
    objectiveMain.Run();
}
#endif

ObjectiveMainENETThreaded::ObjectiveMainENETThreaded(int argc, const char **argv)
{
    std::string compileDate(__DATE__);
    std::string compileTime(__TIME__);
    m_argparse.Initialise(argc, argv, "ObjectiveMainENETThreaded command line interface to GaitSym2019 build "s + compileDate + " "s + compileTime, 0, 0);
    m_argparse.AddArgument("-ow"s, "--outputWarehouse"s, "Output warehouse filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-iw"s, "--inputWarehouse"s, "Input warehouse filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-ms"s, "--modelState"s, "Model state filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-rt"s, "--runTimeLimit"s, "Run time limit"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-st"s, "--simulationTimeLimit"s, "Simulation time limit"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mc"s, "--outputModelStateAtCycle"s, "Output model state at this cycle"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mt"s, "--outputModelStateAtTime"s, "Output model state at this cycle"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mw"s, "--outputModelStateAtWarehouseDistance"s, "Output model state at this warehouse distance"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-wd"s, "--warehouseFailDistanceAbort"s, "Abort the simulation when the warehouse distance fails"s, "0"s, 1, false, ArgParse::Bool);
    m_argparse.AddArgument("-de"s, "--debug"s, "Turn debugging on"s);

    m_argparse.AddArgument("-ol"s, "--outputList"s, "List of objects to produce output"s, ""s, 1, MAX_ARGS, false, ArgParse::String);

    m_argparse.AddArgument("-hl"s, "--hostsList"s, "List of hosts "s, "localhost:8086"s, 1, MAX_ARGS, true, ArgParse::String);
    m_argparse.AddArgument("-to"s, "--timeout"s, "The timeout value in milliseconds"s, "100000"s, 1, false, ArgParse::Int);

    int err = m_argparse.Parse();
    if (err)
    {
        m_argparse.Usage();
        exit(1);
    }

    m_argparse.Get("--outputList"s, &m_outputList);
    m_argparse.Get("--runTimeLimit"s, &m_runTimeLimit);
    m_argparse.Get("--outputModelStateAtTime"s, &m_outputModelStateAtTime);
    m_argparse.Get("--outputModelStateAtCycle"s, &m_outputModelStateAtCycle);
    m_argparse.Get("--outputModelStateAtWarehouseDistance"s, &m_outputModelStateAtWarehouseDistance);
    m_argparse.Get("--simulationTimeLimit"s, &m_simulationTimeLimit);
    m_argparse.Get("--warehouseFailDistanceAbort"s, &m_warehouseFailDistanceAbort);
    m_argparse.Get("--modelState"s, &m_outputModelStateFilename);
    m_argparse.Get("--inputWarehouse"s, &m_inputWarehouseFilename);
    m_argparse.Get("--outputWarehouse"s, &m_outputWarehouseFilename);
    m_argparse.Get("--debug"s, &m_debug);
    m_argparse.Get("--verbose"s, &m_verbose);

    std::vector<std::string> rawHosts;
    std::vector<std::string> result;
    m_argparse.Get("--hostsList"s, &rawHosts);
    for (auto &&it: rawHosts)
    {
        pystring::split(it, result, ":"s);
        if (result.size() == 2)
        {
            ThreadedComms::Host h;
            h.host = result[0];
            h.port = GSUtil::Int(result[1]);
            m_hosts.push_back(std::move(h));
        }
    }
    if (m_hosts.size() == 0)
    {
        std::cerr << "Error: no valid hosts found in hostlist \"" << pystring::join(" "s, rawHosts) << "\"\n";
        exit(1);
    }

    // complicated stuff for the random number generator
    std::random_device rd;
    std::mt19937_64::result_type seed = rd();
    m_gen = std::mt19937_64(seed);
    m_distrib = std::uniform_real_distribution<double>(0.5, 1.5);
}

int ObjectiveMainENETThreaded::Run()
{
    int status = 0;
    ThreadedComms threadedComms;
    threadedComms.setDebug(m_debug);
    if (threadedComms.Initialise(m_hosts)) return __LINE__;

    bool finishedFlag = true;
    int timeoutMultiplier = 1;
    double startTime = GSUtil::GetTime();
    while(m_runTimeLimit == 0 || m_runTime <= m_runTimeLimit)
    {
        double ioTime = 0;
        double cpuTime = 0;
        if (finishedFlag)
        {
            double ioStartTime = GSUtil::GetTime();
            ThreadedComms::ReceivePackage package = threadedComms.UnqueueReceivePackage();
            double parseStartTime = GSUtil::GetTime();
            ioTime += (parseStartTime - ioStartTime);
            status = 1;
            if (package.content.size())
            {
                m_genomePackage = package;
                // create the simulation object
                m_simulation = std::make_unique<Simulation>();
                if (m_outputWarehouseFilename.size()) m_simulation->SetOutputWarehouseFile(m_outputWarehouseFilename);
                if (m_outputModelStateFilename.size()) m_simulation->SetOutputModelStateFile(m_outputModelStateFilename);
                if (m_outputModelStateAtTime >= 0) m_simulation->SetOutputModelStateAtTime(m_outputModelStateAtTime);
                if (m_outputModelStateAtCycle >= 0) m_simulation->SetOutputModelStateAtCycle(m_outputModelStateAtCycle);
                if (m_inputWarehouseFilename.size()) m_simulation->AddWarehouse(m_inputWarehouseFilename);
                if (m_outputModelStateAtWarehouseDistance >= 0) m_simulation->SetOutputModelStateAtWarehouseDistance(m_outputModelStateAtWarehouseDistance);
                if (m_simulation->LoadModel(package.content.c_str(), package.content.size()))
                {
                    m_simulation.reset();
                }
                else
                {
                    // late initialisation options
                    if (m_simulationTimeLimit >= 0) m_simulation->SetTimeLimit(m_simulationTimeLimit);
                    if (m_warehouseFailDistanceAbort != 0) m_simulation->SetWarehouseFailDistanceAbort(m_warehouseFailDistanceAbort);
                    status = 0;
                }
            }
            if (status == 0)
            {
                finishedFlag = false;
                timeoutMultiplier = 1;
                for (size_t i = 0; i < m_outputList.size(); i++)
                {
                    NamedObject *namedObject = m_simulation->GetNamedObject(m_outputList[i]);
                    if (namedObject) namedObject->setDump(true);
                }
                cpuTime += (GSUtil::GetTime() - parseStartTime);
            }
            else
            {
                m_sleepTime = int(m_distrib(m_gen) * 1000.0 * timeoutMultiplier);
                if (m_debug) std::cerr <<  "timeoutMultiplier = " << timeoutMultiplier << " m_sleepTime = " << m_sleepTime << " ms\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepTime));
                if (timeoutMultiplier < 100) timeoutMultiplier++;
                ioTime += (GSUtil::GetTime() - parseStartTime);
            }
        }
        else
        {
            double cpuStartTime = GSUtil::GetTime();
            while (m_simulation->ShouldQuit() == false)
            {
                m_simulation->UpdateSimulation();
                if (m_simulation->TestForCatastrophy()) break;
            }

            finishedFlag = true;
            ThreadedComms::SendPackage sendPackage;
            sendPackage.message = m_genomePackage.message;
            sendPackage.message.score = m_simulation->CalculateInstantaneousFitness();
            sendPackage.host = m_genomePackage.host;
            if (m_verbose) std::cerr << "Simulation Time: " << m_simulation->GetTime() << " Steps: " << m_simulation->GetStepCount()
                                     << " Score: " << sendPackage.message.score << " Mechanical Energy: " << m_simulation->GetMechanicalEnergy()
                                     << " Metabolic Energy: " << m_simulation->GetMetabolicEnergy() << "\n";
            m_simulation.reset();
            double ioStartTime = GSUtil::GetTime();
            cpuTime += (ioStartTime - cpuStartTime);
            threadedComms.QueueSendPackage(sendPackage);
            ioTime += (GSUtil::GetTime() - ioStartTime);
        }
        m_runTime = GSUtil::GetTime() - startTime;
        m_IOTime += ioTime;
        m_simulationTime += cpuTime;
        if (m_verbose) std::cerr << "RunTime: " << m_runTime << " CPUTimeSimulation: " << m_simulationTime << " CPUTimeIO: " << m_IOTime << "\n";
    }
    return 0;
}

ThreadedComms::ThreadedComms()
{
    std::random_device rd;
    std::mt19937_64::result_type seed = rd();
    m_genReceive = std::mt19937_64(seed);
    m_distribReceive = std::uniform_real_distribution<double>(0.5, 1.5);
    seed = rd();
    m_genSend = std::mt19937_64(seed);
    m_distribSend = std::uniform_real_distribution<double>(0.5, 1.5);
}

ThreadedComms::~ThreadedComms()
{
    abortThread = true;
    m_sendThread.join();
    m_receiveThread.join();
    if (m_client) enet_host_destroy(m_client);
    if (m_enetInitialised) enet_deinitialize();
}

int ThreadedComms::Initialise(const std::vector<Host> &hosts)
{
    assert(m_enetInitialised == false);
    m_hosts = hosts;

    int status = enet_initialize();
    if (status)
    {
        std::cerr << "An error occurred while initializing ENet: status = " << status << "\n";
        return __LINE__;
    }
    m_enetInitialised = true;
    size_t peerCount = 1;
    size_t channelLimit = 2;
    enet_uint32 incomingBandwidth = 0;
    enet_uint32 outgoingBandwidth = 0;
    m_client = enet_host_create (nullptr            /* address is zero in client */,
                                 peerCount          /* allow up to 1 clients and/or outgoing connections */,
                                 channelLimit       /* allow up to 2 channels to be used, 0 and 1 */,
                                 incomingBandwidth  /* assume any amount of incoming bandwidth */,
                                 outgoingBandwidth  /* assume any amount of outgoing bandwidth */);
    if (!m_client)
    {
        std::cerr << "Error: could not create the client\n";
        return __LINE__;
    }
    // lookup all the hosts
    for (size_t i = 0; i < m_hosts.size(); i++)
    {

        ENetAddress address;
        int status = enet_address_set_host(&address, m_hosts[i].host.c_str());
        if (status)
        {
            std::cerr << "ThreadedComms::Initialise Host " << i << " " << m_hosts[i].host << " not parsed\n";
            return __LINE__;
        }
        address.port = enet_uint16(m_hosts[i].port);
        m_hosts[i].enetAddress = address;
    }

    m_receiveThread = std::thread(&ThreadedComms::ReceiveThread, this);
    m_sendThread = std::thread(&ThreadedComms::SendThread, this);
    return 0;
}

void ThreadedComms::SendThread()
{
    while (!abortThread)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(uint64_t(m_sendPause * m_sendPauseMultiplier * m_distribSend(m_genSend))));
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_sendQueue.size() == 0)
        {
            if (m_debug) std::cerr <<  "SendThread queue empty\n";
            continue;
        }
        m_genomeMessage = m_sendQueue.front().message;
        m_outputHost = m_sendQueue.front().host;
        int status = WriteOutput();
        if (status)
        {
            m_sendQueue.front().retryCount++;
            m_sendPauseMultiplier *= m_sendPauseMultiplierInflate;
            if (m_sendPauseMultiplier > m_sendPauseMultiplierMax) m_sendPauseMultiplier = m_sendPauseMultiplierMax;
        }
        if (status == 0 || m_sendQueue.front().retryCount > 10)
        {
            m_sendQueue.pop_front();
            m_sendPauseMultiplier = 1;
        }
    }
}

void ThreadedComms::ReceiveThread()
{
    while (!abortThread)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(uint64_t(m_receivePause * m_receivePauseMultiplier * m_distribReceive(m_genReceive))));
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_receiveQueue.size() >= m_receiveQueueMaxSize)
        {
            if (m_debug) std::cerr <<  "ReceiveThread queue full\n";
            continue;
        }
        std::string xml = ReadModel();
        if (xml.size())
        {
            m_receiveQueue.emplace_back();
            m_receiveQueue.back().message = m_genomeMessage;
            m_receiveQueue.back().content = std::move(xml);
            m_receiveQueue.back().host = m_hosts[m_currentHost];
            m_receivePauseMultiplier = 1;
        }
        else
        {
            m_receivePauseMultiplier *= m_receivePauseMultiplierInflate;
            if (m_receivePauseMultiplier > m_receivePauseMultiplierMax) m_receivePauseMultiplier = m_receivePauseMultiplierMax;
        }
    }
}

std::string ThreadedComms::ReadModel()
{
    if (m_debug) std::cerr <<  "ReadModel m_currentHost " << m_currentHost << " host " << m_hosts[m_currentHost].host << " port " << m_hosts[m_currentHost].port << "\n";

    ENetAddress address = m_hosts[m_currentHost].enetAddress;

    size_t channelCount = 1;
    enet_uint32 data = 0;
    if (m_peer) enet_peer_reset(m_peer);
    m_peer = enet_host_connect(m_client, &address, channelCount, data);
    if (!m_peer)
    {
        std::cerr << "ReadModel Host " << m_currentHost << " " << m_hosts[m_currentHost].host << " not available\n";
        m_currentHost++;
        if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
        return std::string();
    }
    else
    {
        if (m_debug) std::cerr <<  "ReadModel enet_host_connect initiated\n" ;
    }

    ENetEvent event = {};
    enet_uint32 timeout = m_timeout; // milliseconds or set to 0 to return immediately
    // first check any incoming events (this call creates an event.packet that we need to destroy later)
    int status = enet_host_service(m_client, &event, timeout);
    if (status > 0)
    {
        switch(event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            m_connected = true;
            if (m_debug) std::cerr << "New connection from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if (m_debug) std::cerr << "Data received from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            if (m_debug) std::cerr << "Data packet is " << event.packet->dataLength << " bytes long\n";
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            if (m_debug) std::cerr << "Disconnect from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            break;
        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }

    // request a new genome from the server
    TCPIPMessage message = {};
    strcpy(message.text, "reqjob");
    message.genomeLength = 0;
    message.runID = 0;
    std::copy(std::begin(m_MD5), std::end(m_MD5), std::begin(message.md5));
    message.senderIP = m_client->address.host;
    message.senderPort = m_client->address.port;
    message.score = 0;
    enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE; // zero or ENET_PACKET_FLAG_RELIABLE most commonly
    ENetPacket *packet = enet_packet_create(message.text, sizeof(TCPIPMessage), flags);
    enet_uint8 channelID = 0;
    status = enet_peer_send(m_peer, channelID, packet);
    if (status)
    {
        std::cerr << "Message " << message.text << " not sent\n";
        return std::string();
    }
    else
    {
        if (m_debug) std::cerr << "Message " << message.text << " sent to " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
    }
    enet_host_flush(m_client);
    std::vector<double> genomeData;
    // wait for a response
    status = enet_host_service(m_client, &event, timeout);
    if (status > 0 && event.type == ENET_EVENT_TYPE_RECEIVE)
    {
        TCPIPMessage *messagePtr = reinterpret_cast<TCPIPMessage *>(event.packet->data);
        double *doublePtr = reinterpret_cast<double *>(event.packet->data + sizeof(TCPIPMessage));
        size_t lenGenome = (event.packet->dataLength - sizeof(TCPIPMessage)) / sizeof(double);
        genomeData.reserve(lenGenome);
        std::copy_n(doublePtr, lenGenome, std::back_inserter(genomeData));
        m_genomeMessage = *reinterpret_cast<TCPIPMessage *>(event.packet->data);
        if (m_debug)
        {
            std::cerr << "Message " << messagePtr->text << " received from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            for (size_t i = 0; i < genomeData.size(); i += 10)
            {
                std::cerr << static_cast<unsigned long>(i);
                for (size_t j = 0; j < 10; j++)
                {
                    if (i + j >= genomeData.size()) break;
                    std::cerr << " " << genomeData[i + j];
                }
                std::cerr << "\n";
            }
        }
        enet_packet_destroy(event.packet);
    }
    else
    {
        std::cerr << "Genome data not received\n";
        return std::string();
    }

    if (!genomeData.size())
    {
        std::cerr << "Host " << m_currentHost << " no genome data sent\n";
        m_currentHost++;
        if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
        return std::string();
    }

    // check the current hash
    if (std::equal(std::begin(m_MD5), std::end(m_MD5), std::begin(m_genomeMessage.md5)) == false)
    {
        // is it in the cache?
        std::vector<uint32_t> hash(m_genomeMessage.md5, m_genomeMessage.md5 + 4);
        auto it = m_cachedConfigFiles.find(hash);
        if (it != m_cachedConfigFiles.end())
        {
            // yes, so just load it from the cache
            std::copy(std::begin(m_genomeMessage.md5), std::end(m_genomeMessage.md5), std::begin(m_MD5));
            m_XMLConverter.LoadBaseXMLString(it->second.c_str(), it->second.size());
        }
        else
        {
            // no, so request it from the server

            strcpy(message.text, "reqxml");
            packet = enet_packet_create(message.text, sizeof(TCPIPMessage), flags);
            status = enet_peer_send(m_peer, channelID, packet);
            if (status)
            {
                std::cerr << "Message " << message.text << " not sent\n";
                return std::string();
            }
            else
            {
                if (m_debug) std::cerr << "Message " << message.text << " sent to " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            }
            enet_host_flush(m_client);
            // wait for a response
            status = enet_host_service(m_client, &event, timeout);
            if (status > 0 && event.type == ENET_EVENT_TYPE_RECEIVE)
            {
                TCPIPMessage *messagePtr = reinterpret_cast<TCPIPMessage *>(event.packet->data);
                hash = std::vector<uint32_t>(messagePtr->md5, messagePtr->md5 + 4);
                size_t lenXML = event.packet->dataLength - sizeof(TCPIPMessage);
                std::string xml(reinterpret_cast<char *>(event.packet->data + sizeof(TCPIPMessage)), lenXML);
                m_XMLConverter.LoadBaseXMLString(xml.c_str(), xml.size());
                std::copy(std::begin(messagePtr->md5), std::end(messagePtr->md5), std::begin(m_MD5));
                m_cachedConfigFiles[hash] = std::move(xml);
                m_cachedConfigFilesQueue.push_back(hash);
                if (m_cachedConfigFilesQueue.size() > m_cachedConfigFilesLimit)
                {
                    m_cachedConfigFiles.erase(m_cachedConfigFilesQueue.front());
                    m_cachedConfigFilesQueue.pop_front();
                }
                if (std::equal(std::begin(m_MD5), std::end(m_MD5), std::begin(m_genomeMessage.md5)) == false)
                {
                    std::cerr << "XML hash does not match " << std::string(hexDigest(messagePtr->md5)) << " != " << std::string(hexDigest(m_genomeMessage.md5)) << "\n";
                    m_currentHost++;
                    if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
                    return std::string();
                }
                if (m_debug)
                {
                    std::cerr << "Message " << messagePtr->text << " received from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
                    std::cerr << m_cachedConfigFiles[hash];
                }
                enet_packet_destroy(event.packet);
            }
            else
            {
                std::cerr << "Host " << m_currentHost << " no xml data sent\n";
                m_currentHost++;
                if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
                return std::string();
            }
        }
    }

    // and apply the new genome
    m_XMLConverter.ApplyGenome(int(genomeData.size()), genomeData.data());
    std::string retVal;
    m_XMLConverter.GetFormattedXML(&retVal);
    return retVal;
}

int ThreadedComms::WriteOutput()
{
    if (m_debug) std::cerr <<  "WriteOutput m_currentHost " << m_currentHost << " host " << m_hosts[m_currentHost].host << " port " << m_hosts[m_currentHost].port << "\n";

    ENetAddress address = m_outputHost.enetAddress;

    size_t channelCount = 1;
    enet_uint32 data = 0;
    if (m_peer) enet_peer_reset(m_peer);
    m_peer = enet_host_connect(m_client, &address, channelCount, data);
    if (!m_peer)
    {
        std::cerr << "WriteOutput Host " << m_currentHost << " " << m_hosts[m_currentHost].host << " not available\n";
        m_currentHost++;
        if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
        return __LINE__;
    }
    else
    {
        if (m_debug) std::cerr <<  "WriteOutput enet_host_connect initiated\n" ;
    }

    ENetEvent event = {};
    enet_uint32 timeout = m_timeout; // milliseconds or set to 0 to return immediately
    // first check any incoming events (this call creates and event.packet that we need to destroy later)
    int status = enet_host_service(m_client, &event, timeout);
    if (status > 0)
    {
        switch(event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            std::cerr << "New connection from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            std::cerr << "Data received from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            std::cerr << "Data packet is " << event.packet->dataLength << " bytes long\n";
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            std::cerr << "Disconnect from " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
            break;
        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }

    strcpy(m_genomeMessage.text, "result");
    enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE; // zero of ENET_PACKET_FLAG_RELIABLE most commonly
    ENetPacket *packet = enet_packet_create(m_genomeMessage.text, sizeof(TCPIPMessage), flags);
    enet_uint8 channelID = 0;
    status = enet_peer_send(m_peer, channelID, packet);
    if (status)
    {
        std::cerr << "Message " << m_genomeMessage.text << " not sent\n";
        return __LINE__;
    }
    else
    {
        if (m_debug) std::cerr << "Message " << m_genomeMessage.text << " sent to " << GSUtil::ToString(event.peer->address.host, event.peer->address.port) << "\n";
    }
    enet_host_flush(m_client);

    return 0;
}

void ThreadedComms::setDebug(bool debug)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_debug = debug;
}

ThreadedComms::ReceivePackage ThreadedComms::UnqueueReceivePackage()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_receiveQueue.size() == 0) return ReceivePackage();
    ReceivePackage result = std::move(m_receiveQueue.front());
    m_receiveQueue.pop_front();
    return result;
}

void ThreadedComms::QueueSendPackage(const ThreadedComms::SendPackage &sendPackage)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_sendQueue.push_back(sendPackage);
}

