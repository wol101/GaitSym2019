/*
 *  ObjectiveMainUDP.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef OBJECTIVEMAINUDP_H
#define OBJECTIVEMAINUDP_H

#include "XMLConverter.h"
#include "ArgParse.h"
#include "ThreadedUDP.h"

#include <string>
#include <vector>
#include <random>
#include <deque>
#include <map>
#include <memory>

class Simulation;

class ObjectiveMainUDP
{
public:
    ObjectiveMainUDP(int argc, const char **argv);

    int Run();

    static int LookupHostname(const std::string &hostname, uint16_t port, struct sockaddr_in *hostAddr);

private:
    int ReadGenome();
    int ReadXML();
    int WriteOutput();

    std::vector<std::string> m_outputList;

    std::unique_ptr<Simulation> m_simulation;
    double m_runTimeLimit = 0;
    double m_simulationTime = 0;
    double m_IOTime = 0;
    double m_outputModelStateAtTime = -1;
    double m_outputModelStateAtCycle = -1;
    double m_outputModelStateAtWarehouseDistance = -1;
    double m_simulationTimeLimit = -1;
    double m_warehouseFailDistanceAbort = 0;

    std::string m_configFilename;
    std::string m_outputWarehouseFilename;
    std::string m_outputModelStateFilename;
    std::string m_inputWarehouseFilename;
    std::string m_scoreFilename;

    XMLConverter m_XMLConverter;
    ArgParse m_argparse;

    struct Host
    {
        std::string host;
        int port;
    };

    struct DataMessage
    {
        char text[16];
        uint32_t senderIP;
        uint32_t senderPort;
        uint32_t runID;
        uint32_t genomeLength;
        uint32_t xmlLength;
        uint32_t md5[4];
        union
        {
            double genome[1];
            char xml[1];
        } payload;
    };

    std::vector<Host> m_hosts;
    std::vector<struct sockaddr_in> m_sockaddr_in_list;
    size_t m_currentHost = 0;
    std::unique_ptr<UDP> m_UDP;
    uint32_t m_redundancyPercentXML = 0;
    uint32_t m_redundancyPercentGenome = 0;
    int m_sleepTime = 10000;

    std::map<std::vector<uint32_t>, std::string> m_cachedConfigFiles;
    std::deque<std::vector<uint32_t>> m_cachedConfigFilesQueue;
    std::vector<uint32_t> m_hash = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    size_t m_cachedConfigFilesLimit = 10;
    DataMessage m_dataMessage = {};
    bool m_xmlMissing = true;
    uint64_t m_packetID = 1;

    std::mt19937_64 m_gen;
    std::uniform_real_distribution<double> m_distrib;

    bool m_debug = false;
    bool m_useThreading = false;
};

#endif // OBJECTIVEMAINUDP_H
