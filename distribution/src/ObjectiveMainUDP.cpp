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

#include "pystring.h"

#include <chrono>
#include <thread>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <netdb.h>
#endif

#define MAX_ARGS 4096

using namespace std::string_literals;

#if defined(USE_UDP)
int main(int argc, const char **argv)
{
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
    m_argparse.AddArgument("-co"s, "--config"s, "Config filename"s, ""s, 1, true, ArgParse::String);
    m_argparse.AddArgument("-ow"s, "--outputWarehouse"s, "Output warehouse filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-iw"s, "--inputWarehouse"s, "Input warehouse filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-ms"s, "--modelState"s, "Model state filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-rt"s, "--runTimeLimit"s, "Run time limit"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-st"s, "--simulationTimeLimit"s, "Simulation time limit"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mc"s, "--outputModelStateAtCycle"s, "Output model state at this cycle"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mt"s, "--outputModelStateAtTime"s, "Output model state at this cycle"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mw"s, "--outputModelStateAtWarehouseDistance"s, "Output model state at this warehouse distance"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-wd"s, "--warehouseFailDistanceAbort"s, "Abort the simulation when the warehouse distance fails"s, "0"s, 1, false, ArgParse::Bool);

    m_argparse.AddArgument("-ol"s, "-outputList"s, "List of objects to produce output"s, ""s, 1, MAX_ARGS, false, ArgParse::String);

    m_argparse.AddArgument("-rp"s, "--redundancyPercent"s, "Percentage redundancy in messages"s, ""s, 1, false, ArgParse::Int);
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

    m_argparse.Get("--redundancyPercent"s, &m_redundancyPercent);
    std::vector<std::string> rawHosts;
    std::vector<std::string> result;
    m_argparse.Get("--hostsList"s, &rawHosts);
    for (auto &&it: rawHosts)
    {
        pystring::split(it, result, ":"s);
        if (result.size() == 2)
        {
            Hosts h;
            h.host = result[0];
            h.port = GSUtil::Int(result[1]);
            m_hosts.push_back(h);
        }
    }
}

int ObjectiveMainUDP::Run()
{
    if ((m_UDP.StartListener(0)) == -1)
    {
        std::cerr << "Error setting up listener\n";
        return 1;
    }
    if ((m_UDP.StartTalker()) == -1)
    {
        std::cerr << "Error setting up talker\n";
        return 1;
    }

    double startTime = GSUtil::GetTime();
    bool finishedFlag = true;
    double currentTime;
    double lastTime = 0;
    double runTime = 0;
    while(m_runTimeLimit == 0 || runTime <= m_runTimeLimit)
    {
        runTime = GSUtil::GetTime() - startTime;

        if (finishedFlag)
        {
            if (ReadModel() == 0)
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
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::microseconds(m_sleepTime)); // slight pause on read failure
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
            if (WriteOutput()) return 0;
        }
    }
    m_UDP.StopTalker();
    m_UDP.StopListener();
    return 0;
}

// this routine attemps to read the model specification and initialise the simulation
// it returns zero on success
int ObjectiveMainUDP::ReadModel()
{
    DataFile myFile;
    myFile.SetExitOnError(false);

    // get model config file from server
    try
    {
#ifdef USE_GETHOSTBYNAME
        struct hostent *he;
        struct sockaddr_in their_addr;
        if ((he = gethostbyname(m_hosts[m_currentHost].host.c_str())) == nullptr) throw __LINE__;
        their_addr.sin_family = AF_INET; // host byte order
        their_addr.sin_port = htons(u_short(m_hosts[m_currentHost].port)); // short, network byte order
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(their_addr.sin_zero), 0, 8); // zero the rest of the struct
#else
        struct addrinfo hints = {};
        addrinfo* pResultList = nullptr;
        struct sockaddr_in their_addr = {};
        const char*hostname = m_hosts[m_currentHost].host.c_str();

        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        //  hints.ai_flags = AI_NUMERICHOST;  // if you know hostname is a numeric stirng, you can skip the DNS lookup by setting this flag

        int result = getaddrinfo(hostname , nullptr, &hints, &pResultList);

        if (result == 0) memcpy(&their_addr, pResultList->ai_addr, sizeof(their_addr));

        if (pResultList != nullptr) freeaddrinfo(pResultList);
#endif

        m_UDP.BumpUDPPacketID();
        ((RequestSendGenomeUDPPacket *)m_UDP.GetUDPPacket())->type = request_send_genome;
        ((RequestSendGenomeUDPPacket *)m_UDP.GetUDPPacket())->port = m_UDP.GetMyAddress()->sin_port;
        ((RequestSendGenomeUDPPacket *)m_UDP.GetUDPPacket())->packetID = m_UDP.GetUDPPacketID();
        int numBytes;
        if ((numBytes = m_UDP.SendUDPPacket(&their_addr, sizeof(RequestSendGenomeUDPPacket))) == -1) throw __LINE__;

        if (m_UDP.CheckReceiver(100000) != 1) throw __LINE__;

        char *buf;
        if (m_redundancyPercent <= 0)
        {
            if ((numBytes = m_UDP.ReceiveText(&buf, m_UDP.GetUDPPacketID())) == -1)  throw __LINE__;
        }
        else
        {
            if ((numBytes = m_UDP.ReceiveFEC(&buf, m_UDP.GetUDPPacketID(), m_redundancyPercent + 100)) == -1)  throw __LINE__;
        }

        ((GenomeReceivedUDPPacket *)m_UDP.GetUDPPacket())->type = genome_received;
        ((GenomeReceivedUDPPacket *)m_UDP.GetUDPPacket())->port = m_UDP.GetMyAddress()->sin_port;
        ((GenomeReceivedUDPPacket *)m_UDP.GetUDPPacket())->packetID = m_UDP.GetUDPPacketID();
        if ((numBytes = m_UDP.SendUDPPacket(&their_addr, sizeof(GenomeReceivedUDPPacket))) == -1) throw __LINE__;

        myFile.SetRawData(buf, strlen(buf));
        delete [] buf;

    }

    catch (int e)
    {
        m_currentHost++;
        if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
        return e;
    }

    // create the simulation object
    m_simulation = new Simulation();
    if (m_outputWarehouseFilename.size()) m_simulation->SetOutputWarehouseFile(m_outputWarehouseFilename);
    if (m_outputModelStateFilename.size()) m_simulation->SetOutputModelStateFile(m_outputModelStateFilename);
    if (m_outputModelStateAtTime >= 0) m_simulation->SetOutputModelStateAtTime(m_outputModelStateAtTime);
    if (m_outputModelStateAtCycle >= 0) m_simulation->SetOutputModelStateAtCycle(m_outputModelStateAtCycle);
    if (m_inputWarehouseFilename.size()) m_simulation->AddWarehouse(m_inputWarehouseFilename);
    if (m_outputModelStateAtWarehouseDistance >= 0) m_simulation->SetOutputModelStateAtWarehouseDistance(m_outputModelStateAtWarehouseDistance);

    if (m_simulation->LoadModel(myFile.GetRawData(), myFile.GetSize()))
    {
        delete m_simulation;
        m_simulation = nullptr;
        return 1;
    }

    // late initialisation options
    if (m_simulationTimeLimit >= 0) m_simulation->SetTimeLimit(m_simulationTimeLimit);
    if (m_warehouseFailDistanceAbort != 0) m_simulation->SetWarehouseFailDistanceAbort(m_warehouseFailDistanceAbort);

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

    try
    {
#ifdef USE_GETHOSTBYNAME
        struct hostent *he;
        struct sockaddr_in their_addr;
        if ((he = gethostbyname(m_hosts[m_currentHost].host.c_str())) == nullptr) throw __LINE__;
        their_addr.sin_family = AF_INET; // host byte order
        their_addr.sin_port = htons(u_short(m_hosts[m_currentHost].port)); // short, network byte order
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(their_addr.sin_zero), 0, 8); // zero the rest of the struct
#else
        struct addrinfo hints = {};
        addrinfo* pResultList = nullptr;
        struct sockaddr_in their_addr = {};
        const char*hostname = m_hosts[m_currentHost].host.c_str();

        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        //  hints.ai_flags = AI_NUMERICHOST;  // if you know hostname is a numeric stirng, you can skip the DNS lookup by setting this flag

        int result = getaddrinfo(hostname , nullptr, &hints, &pResultList);

        if (result == 0) memcpy(&their_addr, pResultList->ai_addr, sizeof(their_addr));

        if (pResultList != nullptr) freeaddrinfo(pResultList);
#endif

        ((SendResultUDPPacket *)m_UDP.GetUDPPacket())->type = send_result;
        ((SendResultUDPPacket *)m_UDP.GetUDPPacket())->result = score;
        ((SendResultUDPPacket *)m_UDP.GetUDPPacket())->port = m_UDP.GetMyAddress()->sin_port;
        ((SendResultUDPPacket *)m_UDP.GetUDPPacket())->packetID = m_UDP.GetUDPPacketID();
        int numBytes;
        if ((numBytes = m_UDP.SendUDPPacket(&their_addr, sizeof(SendResultUDPPacket))) == -1) throw __LINE__;
    }
#pragma warning( suppress : 4101 ) // suppresses 'warning C4101: 'e': unreferenced local variable' for one line
    catch (int e)
    {
#ifdef UDP_DEBUG
        std::cerr <<  "WriteModel error on line " << e << "\n";
#endif
    }

    return 0;
}


