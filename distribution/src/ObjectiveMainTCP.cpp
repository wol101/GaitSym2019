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
    ObjectiveMainTCP objectiveMain(argc, argv);
    objectiveMain.Run();
}
#endif

ObjectiveMainTCP::ObjectiveMainTCP(int argc, const char **argv)
{
    std::string compileDate(__DATE__);
    std::string compileTime(__TIME__);
    m_argparse.Initialise(argc, argv, "ObjectiveMainTCP command line interface to GaitSym2019 build "s + compileDate + " "s + compileTime, 0, 0);
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

int ObjectiveMainTCP::Run()
{
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
    return 0;
}

// this routine attemps to read the model specification and initialise the simulation
// it returns zero on success
int ObjectiveMainTCP::ReadModel()
{
    DataFile myFile;
    myFile.SetExitOnError(false);
#ifdef TCP_DEBUG
    std::cerr <<  "ReadModel m_currentHost " << m_currentHost
        << " host " << m_hosts[m_currentHost].host
        << " port " << m_hosts[m_currentHost].port
        << "\n";
#endif

    // get model config file from server

    int status;
    int numBytes, len;
    char buffer[64];
    struct TCPIPMessage
    {
        char text[32];
        uint32_t length;
        uint32_t runID;
        double score;
        uint32_t md5[4];

        enum { StandardMessageSize = 64 };
    };
    TCPIPMessage *messagePtr = reinterpret_cast<TCPIPMessage *>(buffer);

    try
    {
        status = m_TCP.StartClient(m_hosts[m_currentHost].port, m_hosts[m_currentHost].host.c_str());
        if (status != 0) throw -1 * __LINE__;

        if (m_XMLConverter.BaseXMLString().size() == false)
        {
            strcpy(buffer, "req_xml_length");
            numBytes = m_TCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
            //OUT_VAR(buffer);
            //OUT_VAR(numBytes);
            if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

            numBytes = m_TCP.ReceiveData(buffer, TCPIPMessage::StandardMessageSize, 10, 0);
            //OUT_VAR(numBytes);
            if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;
            len = messagePtr->length;
            //OUT_VAR(len);
            char *xmlbuf = new char[len];

            strcpy(buffer, "req_xml_data");
            numBytes = m_TCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
            //OUT_VAR(buffer);
            if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

            numBytes = m_TCP.ReceiveData(xmlbuf, len, 10, 0);
            //OUT_VAR(numBytes);
            if (numBytes < len) throw __LINE__;
            memcpy(m_MD5, md5(xmlbuf, len), sizeof(m_MD5)); // the is the md5 score of everything that is sent (which includes a terminating zero)
            // std::cerr << hexDigest(gMD5) << "\n";

            m_XMLConverter.LoadBaseXMLString(xmlbuf, len);
            delete [] xmlbuf;
        }

        strcpy(buffer, "req_send_length");
        numBytes = m_TCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

        numBytes = m_TCP.ReceiveData(buffer, TCPIPMessage::StandardMessageSize, 10, 0);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;
        len = messagePtr->length;
        m_submitCount = messagePtr->runID;
        // std::cerr << hexDigest((const unsigned int *)ptr) << "\n";
        if (std::equal(std::begin(m_MD5), std::end(m_MD5), std::begin(messagePtr->md5)) == false)
        {
            m_XMLConverter.Clear();
            throw __LINE__;
        }
        char *buf = new char[len];

        strcpy(buffer, "req_send_data");
        numBytes = m_TCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

        //OUT_VAR(len);
        //OUT_VAR(g_submitCount);

        numBytes = m_TCP.ReceiveData(buf, len, 10, 0);
        if (numBytes < len) throw __LINE__;
        //OUT_VAR(buf);

        double *dPtr = (double *)buf;
        int genomeLength = len / sizeof(double);
        m_XMLConverter.ApplyGenome(genomeLength, dPtr);
        size_t xmlLen;
        const char *xmlPtr = m_XMLConverter.GetFormattedXML(&xmlLen);
        myFile.SetRawData(xmlPtr, xmlLen);
        delete [] buf;
        m_TCP.StopClient();
    }

    catch (int e)
    {
        if (e > 0) m_TCP.StopClient();
#ifdef TCP_DEBUG
        std::cerr <<  "ReadModel error on line " << e << "\n";
#endif
        m_currentHost++;
        if (m_currentHost >= m_hosts.size()) m_currentHost = 0;
        return 1;
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

    int status = 0;
    int numBytes;
    char buffer[64];
    struct TCPIPMessage
    {
        char text[32];
        uint32_t length;
        uint32_t runID;
        double score;
        uint32_t md5[4];

        enum { StandardMessageSize = 64 };
    };
    TCPIPMessage *messagePtr = (TCPIPMessage *)buffer;
    try
    {
        for (int i = 0; i < 10; i++)
        {
            status = m_TCP.StartClient(m_hosts[m_currentHost].port, m_hosts[m_currentHost].host.c_str());
            if (status == 0) break;
            std::this_thread::sleep_for(std::chrono::microseconds(m_sleepAfterFailMicroseconds));
        }
        if (status != 0) throw -1 * __LINE__;

        strcpy(messagePtr->text, "result");
        messagePtr->score = score;
        messagePtr->runID = m_submitCount;
        memcpy(messagePtr->md5, m_MD5, sizeof(m_MD5));
        numBytes = m_TCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;
        m_TCP.StopClient();
    }

    catch (int e)
    {
        std::cerr << "Unable to write result back to host " << m_hosts[m_currentHost].host << " on port " << m_hosts[m_currentHost].port << "\n";
        if (e > 0) m_TCP.StopClient();
        return 1;
    }

    return 0;
}
