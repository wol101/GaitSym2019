/*
 *  ObjectiveMainASIOAsync.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "ObjectiveMainASIOAsync.h"
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

#if defined(USE_ASIO_ASYNC)
int main(int argc, const char **argv)
{
    ObjectiveMainASIOAsync objectiveMain(argc, argv);
    objectiveMain.Run();
}
#endif

ObjectiveMainASIOAsync::ObjectiveMainASIOAsync(int argc, const char **argv)
{
    std::string compileDate(__DATE__);
    std::string compileTime(__TIME__);
    m_argparse.Initialise(argc, argv, "ObjectiveMainASIOAsync command line interface to GaitSym2019 build "s + compileDate + " "s + compileTime, 0, 0);
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

    m_argparse.AddArgument("-ho"s, "--host"s, "Host and port"s, "127.0.0.1:8086"s, 1, true, ArgParse::String);

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

    std::string rawHost;
    std::vector<std::string> result;
    m_argparse.Get("--host"s, &rawHost);
    pystring::split(rawHost, result, ":"s);
    if (result.size() == 2)
    {
        m_host = result[0];
        m_port = uint16_t(GSUtil::Int(result[1]));
    }
    else
    {
        std::cerr << "Error parsing host\n";
        exit(1);
    }


    // complicated stuff for the random number generator
    std::random_device rd;
    std::mt19937_64::result_type seed = rd();
    m_gen = std::mt19937_64(seed);
    m_distrib = std::uniform_real_distribution<double>(0.5, 1.5);
}

int ObjectiveMainASIOAsync::Run()
{
    double startTime = GSUtil::GetTime();
    double runTime = 0;
    double computeTime = 0;
    int status = 0;
    while(m_runTimeLimit == 0 || runTime <= m_runTimeLimit)
    {
        // construct the new thread and run it
        double score = 0;
        uint32_t runID = 0;
        std::string xmlCopy;
        if (m_lastGenomeValid && m_XMLConverter.BaseXMLString().size())
        {
            runID = reinterpret_cast<const DataMessage *>(m_lastGenomeDataMessageRaw.data())->runID;
            if (m_debug) std::cerr <<  "Run runID = " << runID << "\n";
            m_XMLConverter.ApplyGenome(int(reinterpret_cast<const DataMessage *>(m_lastGenomeDataMessageRaw.data())->genomeLength), reinterpret_cast<const DataMessage *>(m_lastGenomeDataMessageRaw.data())->payload.genome);
            m_XMLConverter.GetFormattedXML(&xmlCopy);
        }
        m_statusDoSimulation = __LINE__;
        std::thread simulationThread(&ObjectiveMainASIOAsync::DoSimulation, this, xmlCopy.data(), xmlCopy.size(), &score, &computeTime);

        // while the simulation is running send off the last result and get the new task
        if (m_scoreToSend)
        {
            status = WriteOutput(m_host, m_port, m_lastRunID, m_lastScore);
            if (status && m_debug) std::cerr << "Failed to write output score\n";
            m_scoreToSend = false;
        }
        status = ReadGenome(m_host, m_port, &m_lastGenomeDataMessageRaw);
        if (status) m_lastGenomeValid = false;
        else m_lastGenomeValid = true;
        if (m_lastGenomeValid)
        {
            if (!hashEqual(m_hash.data(), reinterpret_cast<const DataMessage *>(m_lastGenomeDataMessageRaw.data())->md5, m_hash.size()))
            {
                std::string rawMessage;
                ReadXML(m_host, m_port, &rawMessage);
                if (hashEqual(reinterpret_cast<const DataMessage *>(rawMessage.data())->md5, reinterpret_cast<const DataMessage *>(m_lastGenomeDataMessageRaw.data())->md5, m_hash.size()))
                {
                    for (size_t i = 0; i < m_hash.size(); i++) { m_hash[i] = reinterpret_cast<const DataMessage *>(rawMessage.data())->md5[i]; }
                    m_XMLConverter.LoadBaseXMLString(reinterpret_cast<const DataMessage *>(rawMessage.data())->payload.xml, reinterpret_cast<const DataMessage *>(rawMessage.data())->xmlLength);
                }
                else
                {
                    m_lastGenomeValid = false;
                }
            }
        }

        // wait for the simulation thread
        simulationThread.join();
        if (m_statusDoSimulation == 0)
        {
            m_scoreToSend = true;
            m_lastScore = score;
            m_lastRunID = runID;
        }
        else
        {
            m_scoreToSend = false;
        }

        runTime = GSUtil::GetTime() - startTime;
        double housekeeping = runTime - computeTime;
        double utilisation = computeTime / runTime;
        std::cerr << "runTime: " << runTime << " computeTime: " << computeTime << " housekeeping: " << housekeeping << " utilisation: " << utilisation * 100.0 << "%\n";
    }
    return 0;
}

void ObjectiveMainASIOAsync::DoSimulation(const char *xmlPtr, size_t xmlLen, double *score, double *computeTime)
{
    if (xmlLen == 0)
    {
        m_statusDoSimulation = __LINE__;
        return;
    }
    m_statusDoSimulation = 0;

    double startTime = GSUtil::GetTime();

    // create the simulation object
    m_simulation = std::make_unique<Simulation>();
    if (m_outputWarehouseFilename.size()) m_simulation->SetOutputWarehouseFile(m_outputWarehouseFilename);
    if (m_outputModelStateFilename.size()) m_simulation->SetOutputModelStateFile(m_outputModelStateFilename);
    if (m_outputModelStateAtTime >= 0) m_simulation->SetOutputModelStateAtTime(m_outputModelStateAtTime);
    if (m_outputModelStateAtCycle >= 0) m_simulation->SetOutputModelStateAtCycle(m_outputModelStateAtCycle);
    if (m_inputWarehouseFilename.size()) m_simulation->AddWarehouse(m_inputWarehouseFilename);
    if (m_outputModelStateAtWarehouseDistance >= 0) m_simulation->SetOutputModelStateAtWarehouseDistance(m_outputModelStateAtWarehouseDistance);

    if (m_simulation->LoadModel(xmlPtr, xmlLen))
    {
        m_statusDoSimulation = __LINE__;
        return;
    }

    // late initialisation options
    if (m_simulationTimeLimit >= 0) m_simulation->SetTimeLimit(m_simulationTimeLimit);
    if (m_warehouseFailDistanceAbort != 0) m_simulation->SetWarehouseFailDistanceAbort(m_warehouseFailDistanceAbort);
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

    while (m_simulation->ShouldQuit() == false)
    {
        m_simulation->UpdateSimulation();
        if (m_simulation->TestForCatastrophy()) break;
    }
    *score = m_simulation->CalculateInstantaneousFitness();
    std::cerr << "Simulation Time: " << m_simulation->GetTime() <<
                 " Steps: " << m_simulation->GetStepCount() <<
                 " Score: " << *score <<
                 " Mechanical Energy: " << m_simulation->GetMechanicalEnergy() <<
                 " Metabolic Energy: " << m_simulation->GetMetabolicEnergy() <<
                 "\n";
    *computeTime += (GSUtil::GetTime() - startTime);
}

// this routine attemps to read the model specification and initialise the simulation
// it returns zero on success
int ObjectiveMainASIOAsync::ReadGenome(std::string host, uint16_t port, std::string *rawMessage)
{
    if (m_debug) std::cerr <<  "ReadGenome  host " << host << " port " << port << "\n";

    m_timeout = std::chrono::milliseconds(int(10000 * m_distrib(m_gen)));
    try
    {
        m_asioClient.connect(host, std::to_string(port), m_timeout);
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr <<  "ReadGenome m_asioClient.connect() OK\n";

    // request info from the server
    RequestMessage m_requestMessage = {};
    m_requestMessage.senderIP = m_asioClient.socket().local_endpoint().address().to_v4().to_uint();
    m_requestMessage.senderPort = m_asioClient.socket().local_endpoint().port();
    strcpy(m_requestMessage.text, "req_gen_");
    try
    {
        std::string encodedLine = encode(std::string(reinterpret_cast<char *>(&m_requestMessage), sizeof(RequestMessage)));
        m_asioClient.writeBuffer(encodedLine.data(), encodedLine.size(), m_timeout);
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr <<  "ReadGenome req_gen_ sent\n";

    std::string reply;
    try
    {
        reply =  decode(m_asioClient.readLine(m_timeout, '\0'));
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr << "ReadGenome genome received " << reply.size() << " characters\n";
    if (reply.size() < sizeof(DataMessage))
    {
        std::cerr << "ReadGenome reply.size() < sizeof(DataMessage)\n";
        return __LINE__;
    }
    const DataMessage *dataMessagePtr = reinterpret_cast<const DataMessage *>(reply.data());
    if (m_debug) std::cerr << "ReadGenome " << dataMessagePtr->text << " received\n"
                           << "senderIP = " << dataMessagePtr->senderIP
                           << " senderPort = " << dataMessagePtr->senderPort
                           << " runID = " << dataMessagePtr->runID
                           << " genomeLength = " << dataMessagePtr->genomeLength
                           << " xmlLength = " << dataMessagePtr->xmlLength
                           << " md5 = " << dataMessagePtr->md5[0] << " " << dataMessagePtr->md5[1] << " "
                           << dataMessagePtr->md5[2] << " " << dataMessagePtr->md5[3] << "\n";
    if (reply.size() < sizeof(DataMessage) + dataMessagePtr->genomeLength * sizeof(double))
    {
        std::cerr << "ReadGenome reply.size() < sizeof(DataMessage) + dataMessagePtr->genomeLength * sizeof(double)\n";
        return __LINE__;
    }
    *rawMessage = reply;
    return 0;
}

int ObjectiveMainASIOAsync::ReadXML(std::string host, uint16_t port, std::string *rawMessage)
{
    if (m_debug) std::cerr <<  "ReadXML host " << host << " port " << port << "\n";

    m_timeout = std::chrono::milliseconds(int(10000 * m_distrib(m_gen)));
    try
    {
        m_asioClient.connect(host, std::to_string(port), m_timeout);
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr <<  "ReadXML m_asioClient.connect() OK\n";

    RequestMessage requestMessage = {};
    requestMessage.senderIP = m_asioClient.socket().local_endpoint().address().to_v4().to_uint();
    requestMessage.senderPort = m_asioClient.socket().local_endpoint().port();
    strcpy(requestMessage.text, "req_xml_");
    try
    {
        std::string encodedLine = encode(std::string(reinterpret_cast<char *>(&requestMessage), sizeof(RequestMessage)));
        m_asioClient.writeBuffer(encodedLine.data(), encodedLine.size(), m_timeout);
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr << "ReadXML req_xml_ sent\n";

    std::string reply;
    try
    {
        reply =  decode(m_asioClient.readLine(m_timeout, '\0'));
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr << "ReadXML xml received " << reply.size() << " characters\n";
    if (reply.size() < sizeof(DataMessage))
    {
        std::cerr << "ReadXML reply.size() < sizeof(DataMessage)\n";
        return __LINE__;
    }
    const DataMessage *dataMessagePtr = reinterpret_cast<const DataMessage *>(reply.data());
    if (reply.size() < sizeof(DataMessage) + dataMessagePtr->xmlLength * sizeof(char))
    {
        std::cerr << "ReadXML reply.size() < sizeof(DataMessage) + dataMessagePtr->xmlLength * sizeof(char)\n";
        return __LINE__;
    }

    *rawMessage = reply;
    return 0;
}

// returns 0 if continuing
// returns 1 if exit requested
int ObjectiveMainASIOAsync::WriteOutput(std::string host, uint16_t port, uint32_t runID, double score)
{
    m_timeout = std::chrono::milliseconds(int(100000 * m_distrib(m_gen)));
    try
    {
        m_asioClient.connect(host, std::to_string(port), m_timeout);
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr <<  "WriteOutput m_asioClient.connect() OK\n";

    RequestMessage requestMessage = {};
    requestMessage.senderIP = m_asioClient.socket().local_endpoint().address().to_v4().to_uint();
    requestMessage.senderPort = m_asioClient.socket().local_endpoint().port();
    strcpy(requestMessage.text, "score___");
    requestMessage.score = score;
    requestMessage.runID = runID;
    try
    {
        std::string encodedString = encode(std::string(reinterpret_cast<char *>(&requestMessage), sizeof(RequestMessage)));
        m_asioClient.writeBuffer(encodedString.data(), encodedString.size(), m_timeout);
    }
    catch (std::exception& e)
    {
        std::cerr << __LINE__ << " " << e.what() << std::endl;
        return __LINE__;
    }
    if (m_debug) std::cerr << "WriteOutput score = " << score << " runID = " << runID << " sent\n";

    return 0;
}

std::string ObjectiveMainASIOAsync::encode(const std::string &input)
{
    std::string output;
    output.reserve(input.size() * 2 + 1);
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] == '\0')
        {
            output.push_back('\xff');
            output.push_back('\x1');
            continue;
        }
        if (input[i] == '\xff')
        {
            output.push_back('\xff');
            output.push_back('\x2');
            continue;
        }
        output.push_back(input[i]);
    }
    output.push_back('\0');
    return output;
}

std::string ObjectiveMainASIOAsync::decode(const std::string &input)
{
    std::string output;
    output.reserve(input.size());
    const char *ptr = input.data();
    while (*ptr)
    {
        if (*ptr != '\xff')
        {
            output.push_back(*ptr);
            ptr++;
            continue;
        }
        ptr++;
        if (*ptr)
        {
            if (*ptr == '\x1')
            {
                output.push_back('\0');
                ptr++;
                continue;
            }
            if (*ptr == '\x2')
            {
                output.push_back('\xff');
                ptr++;
                continue;
            }
        }
    }
    return output;
}

bool ObjectiveMainASIOAsync::hashEqual(const uint32_t *hash1, const uint32_t *hash2, size_t hashSize)
{
    for (size_t i = 0; i < hashSize; i++)
    {
        if (hash1[i] != hash2[i]) return false;
    }
    return true;
}

