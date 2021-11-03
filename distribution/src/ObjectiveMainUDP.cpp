/*
 *  ObjectiveMainUDP.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "ObjectiveMainUDP.h"
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

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <WinSock2.h>
#include <Ws2tcpip.h>
#else
#include <netdb.h>
#endif

#define MAX_ARGS 4096

using namespace std::string_literals;

#if defined(USE_UDP)
int main(int argc, const char **argv)
{
    UDPUpDown udpUpDown;
    if (udpUpDown.status())
    {
        std::cerr << "Error initialising UDP library\n";
        return 1;
    }
    ObjectiveMainUDP objectiveMain(argc, argv);
    objectiveMain.Run();
}
#endif

ObjectiveMainUDP::ObjectiveMainUDP(int argc, const char **argv)
{
    std::string compileDate(__DATE__);
    std::string compileTime(__TIME__);
    m_argparse.Initialise(argc, argv, "ObjectiveMainUDP command line interface to GaitSym2019 build "s + compileDate + " "s + compileTime, 0, 0);
    m_argparse.AddArgument("-sc"s, "--score"s, "Score filename"s, ""s, 1, false, ArgParse::String);
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

    m_argparse.AddArgument("-rx"s, "--redundancyPercentXML"s, "Percentage redundancy in XML receive"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-rg"s, "--redundancyPercentGenome"s, "Percentage redundancy in genome receive"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-hl"s, "--hostsList"s, "List of hosts "s, "localhost:8086"s, 1, MAX_ARGS, true, ArgParse::String);

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
    m_argparse.Get("--config"s, &m_configFilename);
    m_argparse.Get("--score"s, &m_scoreFilename);
    m_argparse.Get("--modelState"s, &m_outputModelStateFilename);
    m_argparse.Get("--inputWarehouse"s, &m_inputWarehouseFilename);
    m_argparse.Get("--outputWarehouse"s, &m_outputWarehouseFilename);
    m_argparse.Get("--debug"s, &m_debug);

    int redundancyPercent;
    m_argparse.Get("--redundancyPercentXML"s, &redundancyPercent);
    m_redundancyPercentXML = uint32_t(redundancyPercent);
    m_argparse.Get("--redundancyPercentGenome"s, &redundancyPercent);
    m_redundancyPercentGenome = uint32_t(redundancyPercent);

    std::vector<std::string> rawHosts;
    std::vector<std::string> result;
    m_argparse.Get("--hostsList"s, &rawHosts);
    for (auto &&it: rawHosts)
    {
        pystring::split(it, result, ":"s);
        if (result.size() == 2)
        {
            Host host;
            host.host = result[0];
            host.port = GSUtil::Int(result[1]);
            struct sockaddr_in hostAddr = {};
            if (LookupHostname(host.host, host.port, &hostAddr)) continue;
            else
            {
                 m_hosts.push_back(std::move(host));
                 m_sockaddr_in_list.push_back(hostAddr);
            }
        }
    }
    if (m_sockaddr_in_list.size() == 0)
    {
        std::cerr << "No valid hosts found\n";
        exit(1);
    }

    // complicated stuff for the random number generator
    std::random_device rd;
    std::mt19937_64::result_type seed = rd();
    m_gen = std::mt19937_64(seed);
    m_distrib = std::uniform_real_distribution<double>(0.5, 1.5);
}

int ObjectiveMainUDP::Run()
{
    if (m_useThreading) m_UDP = std::make_unique<ThreadedUDP>();
    else m_UDP = std::make_unique<UDP>();
    m_UDP->setDebug(m_debug);
    if ((m_UDP->StartListener(0)) == -1)
    {
        std::cerr << "Error setting up listener\n";
        return 1;
    }
    StopListenerGuard stopListener(m_UDP.get());
//    if ((m_UDP->StartTalker()) == -1)
//    {
//        std::cerr << "Error setting up talker\n";
//        return 1;
//    }
//    StopTalkerGuard stopTalker(m_UDP.get());

    double startTime = GSUtil::GetTime();
    bool finishedFlag = true;
    double currentTime;
    double lastTime = 0;
    double runTime = 0;
    double timeoutMultiplier = 1.0;
    while(m_runTimeLimit == 0 || runTime <= m_runTimeLimit)
    {
        runTime = GSUtil::GetTime() - startTime;

        if (finishedFlag)
        {
            if (m_xmlMissing) ReadXML();
            if (m_xmlMissing == false && ReadGenome() == 0)
            {
                finishedFlag = false;

                for (size_t i = 0; i < m_outputList.size(); i++)
                {
                    if (m_simulation->GetBodyList()->find(m_outputList[i]) != m_simulation->GetBodyList()->end()) (*m_simulation->GetBodyList())[m_outputList[i]]->setDump(true);
                    if (m_simulation->GetMuscleList()->find(m_outputList[i]) != m_simulation->GetMuscleList()->end()) (*m_simulation->GetMuscleList())[m_outputList[i]]->setDump(true);
                    if (m_simulation->GetGeomList()->find(m_outputList[i]) != m_simulation->GetGeomList()->end()) (*m_simulation->GetGeomList())[m_outputList[i]]->setDump(true);
                    if (m_simulation->GetJointList()->find(m_outputList[i]) != m_simulation->GetJointList()->end()) (*m_simulation->GetJointList())[m_outputList[i]]->setDump(true);
                    if (m_simulation->GetDriverList()->find(m_outputList[i]) != m_simulation->GetDriverList()->end()) (*m_simulation->GetDriverList())[m_outputList[i]]->setDump(true);
                    if (m_simulation->GetDataTargetList()->find(m_outputList[i]) != m_simulation->GetDataTargetList()->end()) (*m_simulation->GetDataTargetList())[m_outputList[i]]->setDump(true);
                    if (m_simulation->GetReporterList()->find(m_outputList[i]) != m_simulation->GetReporterList()->end()) (*m_simulation->GetReporterList())[m_outputList[i]]->setDump(true);
                }
                timeoutMultiplier = 1.0;
            }
            else
            {
                m_currentHost++;
                if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
                // randomly variable increasing sleep time
                m_sleepTime = int(m_distrib(m_gen) * 1000.0 * timeoutMultiplier);
                if (m_debug) std::cerr <<  "timeoutMultiplier = " << timeoutMultiplier << " m_sleepTime = " << m_sleepTime << " ms\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepTime));
                if (timeoutMultiplier < 100) timeoutMultiplier++;
            }
        }
        else
        {
            currentTime = GSUtil::GetTime();
            m_IOTime += (currentTime - lastTime);
            lastTime = currentTime;
            while (m_simulation->ShouldQuit() == false)
            {
                m_simulation->UpdateSimulation();
                if (m_simulation->TestForCatastrophy()) break;
            }
            currentTime = GSUtil::GetTime();
            m_simulationTime += (currentTime - lastTime);
            lastTime = currentTime;

            finishedFlag = true;
            int status = 0;
            for (size_t i = 0; i < 100; i++) { if ((status = WriteOutput()) == 0) break; }
            if (status && m_debug) std::cerr << "Failed to write output score\n";
            m_simulation.reset();
        }
    }
    return 0;
}

// this routine attemps to read the model specification and initialise the simulation
// it returns zero on success
int ObjectiveMainUDP::ReadGenome()
{
    if (m_debug) std::cerr << "ReadGenome m_currentHost " << m_currentHost << " host " << m_hosts[m_currentHost].host << " port " << m_hosts[m_currentHost].port << "\n";

    // ask for a genome (item = 1)
    udp::RequestItemUDPPacket packet;
    packet.packetID = m_packetID;
    packet.port = m_UDP->GetMyAddress()->sin_port;
    packet.ip4Address = m_UDP->GetMyAddress()->sin_addr.s_addr;
    packet.item = udp::genome;
    packet.redundancy = m_redundancyPercentGenome;
    m_packetID++; if (m_packetID == 0) m_packetID++;
    int numBytes = m_UDP->SendUDPPacket(m_sockaddr_in_list[m_currentHost], &packet, sizeof(udp::RequestItemUDPPacket));
    if (numBytes != sizeof(udp::RequestItemUDPPacket))
    {
        if (m_debug) std::cerr << "ReadGenome() error in SendUDPPacket\n";
        return __LINE__;
    }

    if (m_UDP->CheckReceiver(100000) <= 0)
    {
        if (m_debug) std::cerr << "ReadGenome() timeout in CheckReceiver\n";
        return __LINE__;
    }

    std::string data;
    struct sockaddr_in sender;
    if (m_redundancyPercentGenome == 0) numBytes = m_UDP->ReceiveText(packet.packetID, &data, &sender);
    else numBytes = m_UDP->ReceiveFEC(packet.packetID, m_redundancyPercentGenome, &data, &sender);
    if (numBytes <= 0)
    {
        if (m_debug) std::cerr << "ReadGenome() error in ReceiveText or ReceiveFEC\n";
        return __LINE__;
    }
    if (m_debug) std::cerr << "ReadGenome data received " << data.size() << " characters\n";

    DataMessage *dataMessagePtr = reinterpret_cast<DataMessage *>(const_cast<char *>(data.data())); // yes this will work
    if (m_debug) std::cerr << "ReadGenome " << dataMessagePtr->text << " received\n"
                           << "senderIP = " << dataMessagePtr->senderIP
                           << " senderPort = " << dataMessagePtr->senderPort
                           << " runID = " << dataMessagePtr->runID
                           << " genomeLength = " << dataMessagePtr->genomeLength
                           << " xmlLength = " << dataMessagePtr->xmlLength
                           << " md5 = " << dataMessagePtr->md5[0] << " " << dataMessagePtr->md5[1] << " "
                           << dataMessagePtr->md5[2] << " " << dataMessagePtr->md5[3] << "\n";
    m_dataMessage = *dataMessagePtr;

    // check the current hash
    if (std::equal(std::begin(m_hash), std::end(m_hash), std::begin(dataMessagePtr->md5)) == false)
    {
        // hash doesn't match but is it in the cache?
        for (size_t i = 0; i < m_hash.size(); i++) m_hash[i] = dataMessagePtr->md5[i];
        auto it = m_cachedConfigFiles.find(m_hash);
        if (it != m_cachedConfigFiles.end())
        {
            if (m_debug) std::cerr << "XML file found in cache\n";
            m_XMLConverter.LoadBaseXMLString(it->second.c_str(), it->second.size());
        }
        else
        {
            if (m_debug) std::cerr << "XML file not found in cache\n";
            m_xmlMissing = true;
            return __LINE__;
        }
    }

    // and apply the new genome
    m_XMLConverter.ApplyGenome(int(dataMessagePtr->genomeLength), dataMessagePtr->payload.genome);
    std::string xmlString;
    m_XMLConverter.GetFormattedXML(&xmlString);

    // create the simulation object
    m_simulation = std::make_unique<Simulation>();
    if (m_outputWarehouseFilename.size()) m_simulation->SetOutputWarehouseFile(m_outputWarehouseFilename);
    if (m_outputModelStateFilename.size()) m_simulation->SetOutputModelStateFile(m_outputModelStateFilename);
    if (m_outputModelStateAtTime >= 0) m_simulation->SetOutputModelStateAtTime(m_outputModelStateAtTime);
    if (m_outputModelStateAtCycle >= 0) m_simulation->SetOutputModelStateAtCycle(m_outputModelStateAtCycle);
    if (m_inputWarehouseFilename.size()) m_simulation->AddWarehouse(m_inputWarehouseFilename);
    if (m_outputModelStateAtWarehouseDistance >= 0) m_simulation->SetOutputModelStateAtWarehouseDistance(m_outputModelStateAtWarehouseDistance);

    if (m_simulation->LoadModel(xmlString.c_str(), xmlString.size()))
    {
        if (m_debug) std::cerr << "Error loading XML file into simulation\n";
        m_simulation.reset();
        return __LINE__;
    }

    // late initialisation options
    if (m_simulationTimeLimit >= 0) m_simulation->SetTimeLimit(m_simulationTimeLimit);
    if (m_warehouseFailDistanceAbort != 0) m_simulation->SetWarehouseFailDistanceAbort(m_warehouseFailDistanceAbort);

    return 0;
}

int ObjectiveMainUDP::ReadXML()
{
    if (m_debug) std::cerr << "ReadXML m_currentHost " << m_currentHost << " host " << m_hosts[m_currentHost].host << " port " << m_hosts[m_currentHost].port << "\n";

    // ask for xml (item = 2)
    udp::RequestItemUDPPacket packet;
    packet.packetID = m_packetID;
    packet.port = m_UDP->GetMyAddress()->sin_port;
    packet.ip4Address = m_UDP->GetMyAddress()->sin_addr.s_addr;
    packet.item = udp::xml;
    packet.redundancy = m_redundancyPercentXML;
    m_packetID++; if (m_packetID == 0) m_packetID++;
    int numBytes = m_UDP->SendUDPPacket(m_sockaddr_in_list[m_currentHost], &packet, sizeof(udp::RequestItemUDPPacket));
    if (numBytes != sizeof(udp::RequestItemUDPPacket))
    {
        if (m_debug) std::cerr << "ReadXML() error in SendUDPPacket\n";
        return __LINE__;
    }

    if (m_UDP->CheckReceiver(100000) <= 0)
    {
        if (m_debug) std::cerr << "ReadXML() timeout in CheckReceiver\n";
        return __LINE__;
    }


    std::string data;
    struct sockaddr_in sender;
    if (m_redundancyPercentXML == 0) numBytes = m_UDP->ReceiveText(packet.packetID, &data, &sender);
    else numBytes = m_UDP->ReceiveFEC(packet.packetID, m_redundancyPercentXML, &data, &sender);
    if (numBytes <= 0)
    {
        if (m_debug) std::cerr << "ReadXML() error in ReceiveText or ReceiveFEC\n";
        return __LINE__;
    }
    if (m_debug) std::cerr << "ReadXML xml received " << data.size() << " characters\n";

    const DataMessage *dataMessagePtr = reinterpret_cast<const DataMessage *>(data.data());
    if (m_debug) std::cerr << "ReadXML " << dataMessagePtr->text << " received\n"
                           << "senderIP = " << dataMessagePtr->senderIP
                           << " senderPort = " << dataMessagePtr->senderPort
                           << " runID = " << dataMessagePtr->runID
                           << " genomeLength = " << dataMessagePtr->genomeLength
                           << " xmlLength = " << dataMessagePtr->xmlLength
                           << " md5 = " << dataMessagePtr->md5[0] << " " << dataMessagePtr->md5[1] << " "
                           << dataMessagePtr->md5[2] << " " << dataMessagePtr->md5[3] << "\n";
    m_XMLConverter.LoadBaseXMLString(dataMessagePtr->payload.xml, dataMessagePtr->xmlLength);
    std::string xml(dataMessagePtr->payload.xml, dataMessagePtr->xmlLength);
    // check the hash to make sure we have got the right file and that it has not got corrupted
    uint32_t *checkHash = md5(xml.data(), int(xml.size()));
    if (std::equal(dataMessagePtr->md5, dataMessagePtr->md5 + 4, checkHash) == false)
    {
        if (m_debug) std::cerr << "ReadXML local hash does not match\n";
        if (m_debug) std::cerr << xml <<"\n";
        return __LINE__;
    }
    for (size_t i = 0; i < m_hash.size(); i++) m_hash[i] = dataMessagePtr->md5[i];
    m_cachedConfigFiles[m_hash] = std::move(xml);
    m_cachedConfigFilesQueue.push_back(m_hash);

    if (m_cachedConfigFilesQueue.size() > m_cachedConfigFilesLimit)
    {
        m_cachedConfigFiles.erase(m_cachedConfigFilesQueue.front());
        m_cachedConfigFilesQueue.pop_front();
    }
    m_xmlMissing = false;
    return 0;
}

// returns 0 if continuing
// returns 1 if exit requested
int ObjectiveMainUDP::WriteOutput()
{
    double score = m_simulation->CalculateInstantaneousFitness();
    std::cerr << "Simulation Time: " << m_simulation->GetTime() <<
                 " Steps: " << m_simulation->GetStepCount() <<
                 " Score: " << score <<
                 " Mechanical Energy: " << m_simulation->GetMechanicalEnergy() <<
                 " Metabolic Energy: " << m_simulation->GetMetabolicEnergy() <<
                 " CPUTimeSimulation: " << m_simulationTime <<
                 " CPUTimeIO: " << m_IOTime <<
                 "\n";

    udp::RequestItemUDPPacket packet;
    packet.packetID = m_packetID;
    packet.port = m_UDP->GetMyAddress()->sin_port;
    packet.ip4Address = m_UDP->GetMyAddress()->sin_addr.s_addr;
    packet.item = udp::score;
    packet.score = score;
    packet.runID = m_dataMessage.runID;
    m_packetID++; if (m_packetID == 0) m_packetID++;
    int numBytes = m_UDP->SendUDPPacket(m_sockaddr_in_list[m_currentHost], &packet, sizeof(udp::RequestItemUDPPacket));
    if (numBytes != sizeof(udp::RequestItemUDPPacket))
    {
        if (m_debug) std::cerr << "WriteOutput() error in SendUDPPacket\n";
        return __LINE__;
    }
    return 0;
}

int ObjectiveMainUDP::LookupHostname(const std::string &hostname, uint16_t port, struct sockaddr_in *hostAddr)
{
    *hostAddr = {};
#ifdef USE_GETHOSTBYNAME
    struct hostent *he = gethostbyname(hostname.c_str());
    if (!he) return __LINE__;
    hostAddr->sin_family = AF_INET; // host byte order
    hostAddr->sin_port = htons(port); // short, network byte order
    hostAddr->sin_addr = *((struct in_addr *)he->h_addr);
#else
    struct addrinfo hints = {};
    addrinfo* pResultList = nullptr;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    //  hints.ai_flags = AI_NUMERICHOST;  // if you know hostname is a numeric string, you can skip the DNS lookup by setting this flag
    int result = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &pResultList);
    if (result) return __LINE__;
    if (!pResultList) return __LINE__;
    // pResultList can contain a list of lookup results but we are only interested in the first one
    // only IP4 currently supported
    if (pResultList->ai_family != AF_INET) return __LINE__;
    *hostAddr = *reinterpret_cast<struct sockaddr_in *>(pResultList->ai_addr);
    if (pResultList != nullptr) freeaddrinfo(pResultList);
#endif
    return 0;
}


