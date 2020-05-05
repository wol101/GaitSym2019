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
#include "UDP.h"

#include <string>
#include <vector>

class Simulation;

class ObjectiveMainUDP
{
public:
    ObjectiveMainUDP(int argc, const char **argv);

    int Run();
    int ReadModel();
    int WriteOutput();

private:
    std::vector<std::string> m_outputList;

    Simulation *m_simulation = nullptr;
    double m_runTimeLimit = 0;
    double m_simulationTime = 0;
    double m_IOTime = 0;
    double m_outputModelStateAtTime = -1;
    double m_outputModelStateAtCycle = -1;
    double m_outputModelStateAtWarehouseDistance = -1;
    bool m_mungeModelStateFlag = false;
    bool m_mungeRotationFlag = false;
    double m_simulationTimeLimit = -1;
    double m_warehouseFailDistanceAbort = 0;

    std::string m_configFilename;
    std::string m_outputWarehouseFilename;
    std::string m_outputModelStateFilename;
    std::string m_inputWarehouseFilename;
    std::string m_scoreFilename;

    XMLConverter m_XMLConverter;
    ArgParse m_argparse;

    struct Hosts
    {
        std::string host;
        int port;
    };
    std::vector<Hosts> m_hosts;
    size_t m_currentHost = 0;
    UDP m_UDP;
    int m_redundancyPercent = 0;
    int m_sleepTime = 10000;
};

#endif // OBJECTIVEMAINUDP_H
