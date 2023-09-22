/*
 *  ObjectiveMainTCP.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "ObjectiveMainTCP.h"
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

#if defined(USE_TCP)
int main(int argc, const char **argv)
{
    if (TCP::OneOffInitialisation())
    {
        std::cerr << "Failed to initialise TCPIP\n";
        return 1;
    }
    ObjectiveMainTCP objectiveMain(argc, argv);
    objectiveMain.Run();
    TCP::OneOffCleanup();
}
#endif

ObjectiveMainTCP::ObjectiveMainTCP(int argc, const char **argv)
{
    std::string compileDate(__DATE__);
    std::string compileTime(__TIME__);
    m_argparse.Initialise(argc, argv, "ObjectiveMainTCP command line interface to GaitSym2019 build "s + compileDate + " "s + compileTime, 0, 0);
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

    std::vector<std::string> rawHosts;
    std::vector<std::string> result;
    m_argparse.Get("--hostsList"s, &rawHosts);
    for (auto &&it: rawHosts)
    {
        pystring::split(it, result, ":"s);
        if (result.size() == 2)
        {
            Host h;
            h.host = result[0];
            h.port = GSUtil::Int(result[1]);
            m_hosts.push_back(std::move(h));
        }
    }

    // complicated stuff for the random number generator
    std::random_device rd;
    std::mt19937_64::result_type seed = rd();
    m_gen = std::mt19937_64(seed);
    m_distrib = std::uniform_real_distribution<double>(0.5, 1.5);
}

int ObjectiveMainTCP::Run()
{
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
            if (ReadGenome() == 0)
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
int ObjectiveMainTCP::ReadGenome()
{
    if (m_debug) std::cerr <<  "ReadGenome m_currentHost " << m_currentHost << " host " << m_hosts[m_currentHost].host << " port " << m_hosts[m_currentHost].port << "\n";

    int status = m_TCP.StartClient(m_hosts[m_currentHost].port, m_hosts[m_currentHost].host.c_str());
    if (status != 0) { return __LINE__; }
    if (m_debug) std::cerr <<  "ReadGenome TCP.StartClient initiated\n";
    TCPStopClientGuard stopClientGuard(&m_TCP);

    // request info from the server
    m_requestMessage = {};
    m_TCP.GetMyAddress(&m_requestMessage.senderIP, &m_requestMessage.senderPort);
    strcpy(m_requestMessage.text, "req_genome");
    int numBytes = m_TCP.SendData(m_requestMessage.text, sizeof(RequestMessage));
    if (numBytes != sizeof(RequestMessage)) { return __LINE__; }
    if (m_debug) std::cerr <<  "ReadGenome req_genome sent\n";
    std::vector<char> dataMessage(sizeof(DataMessage) + m_dataMessage.genomeLength * sizeof(double));
    DataMessage *dataMessagePtr = reinterpret_cast<DataMessage *>(dataMessage.data());
    numBytes = m_TCP.ReceiveData(dataMessage.data(), int(dataMessage.size()), 10, 0);
    if (numBytes != int(dataMessage.size())) { return __LINE__; }
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
            // yes, so just load it from the cache
            if (m_debug) std::cerr << "XML file found in cache\n";
            m_XMLConverter.LoadBaseXMLString(it->second.c_str(), it->second.size());
        }
        else
        {
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
        m_simulation.reset();
        return __LINE__;
    }

    // late initialisation options
    if (m_simulationTimeLimit >= 0) m_simulation->SetTimeLimit(m_simulationTimeLimit);
    if (m_warehouseFailDistanceAbort != 0) m_simulation->SetWarehouseFailDistanceAbort(m_warehouseFailDistanceAbort);

    return 0;

}

int ObjectiveMainTCP::ReadXML()
{
    if (m_debug) std::cerr <<  "ReadXML m_currentHost " << m_currentHost << " host " << m_hosts[m_currentHost].host << " port " << m_hosts[m_currentHost].port << "\n";

    int status = m_TCP.StartClient(m_hosts[m_currentHost].port, m_hosts[m_currentHost].host.c_str());
    if (status != 0) { return __LINE__; }
    if (m_debug) std::cerr <<  "ReadGenome TCP.StartClient initiated\n";
    TCPStopClientGuard stopClientGuard(&m_TCP);

    m_requestMessage = {};
    m_TCP.GetMyAddress(&m_requestMessage.senderIP, &m_requestMessage.senderPort);
    strcpy(m_requestMessage.text, "req_xml");
    int numBytes = m_TCP.SendData(m_requestMessage.text, sizeof(RequestMessage));
    if (numBytes != sizeof(RequestMessage)) { return __LINE__; }
    if (m_debug) std::cerr << "ReadXML req_xml sent\n";

    std::vector<char> dataMessage(sizeof(DataMessage) + m_dataMessage.xmlLength * sizeof(char));
    DataMessage *dataMessagePtr = reinterpret_cast<DataMessage *>(dataMessage.data());
    numBytes = m_TCP.ReceiveData(dataMessage.data(), int(dataMessage.size()), 10, 0);
    if (numBytes != int(dataMessage.size())) { return __LINE__; }
    if (m_debug) std::cerr << "ReadXML xml received " << dataMessage.size() << " characters\n";

    m_XMLConverter.LoadBaseXMLString(dataMessagePtr->payload.xml, dataMessagePtr->xmlLength);
    std::string xml(dataMessagePtr->payload.xml, dataMessagePtr->xmlLength);
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
int ObjectiveMainTCP::WriteOutput()
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

    int status = m_TCP.StartClient(m_hosts[m_currentHost].port, m_hosts[m_currentHost].host.c_str());
    if (status != 0) { return __LINE__; }
    if (m_debug) std::cerr <<  "ReadGenome TCP.StartClient initiated\n";
    TCPStopClientGuard stopClientGuard(&m_TCP);

    m_requestMessage = {};
    m_TCP.GetMyAddress(&m_requestMessage.senderIP, &m_requestMessage.senderPort);
    strcpy(m_requestMessage.text, "result");
    m_requestMessage.score = score;
    m_requestMessage.runID = m_dataMessage.runID;
    int numBytes = m_TCP.SendData(m_requestMessage.text, sizeof(RequestMessage));
    if (numBytes != sizeof(RequestMessage))
    {
        std::cerr << "SendData error: Unable to write result back to host " << m_hosts[m_currentHost].host << " on port " << m_hosts[m_currentHost].port << "\n";
        return __LINE__;
    }

    return 0;
}