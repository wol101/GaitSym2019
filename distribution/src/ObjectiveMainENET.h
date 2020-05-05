/*
 *  ObjectiveMainENET.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef OBJECTIVEMAINENET_H
#define OBJECTIVEMAINENET_H


#include "XMLConverter.h"
#include "ArgParse.h"
#include "TCPIPMessage.h"

#include "enet/enet.h"

#include <string>
#include <vector>
#include <map>
#include <deque>
#include <memory>

class Simulation;
class Hosts;

class ObjectiveMainENET
{
public:
    ObjectiveMainENET(int argc, const char **argv);

    int Run();
    int ReadModel();
    int WriteOutput();

private:
    std::vector<std::string> m_outputList;

    std::unique_ptr<Simulation> m_simulation;
    double m_runTimeLimit = 0;
    double m_simulationTime = 0;
    double m_IOTime = 0;
    double m_runTime = 0;
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

    std::vector<std::unique_ptr<Hosts>> m_hosts;
    size_t m_currentHost = 0;
    ENetHost *m_client = nullptr;
    ENetPeer *m_peer = nullptr;
    unsigned int m_MD5[4] = {};
    int m_sleepTime = 10000;

    std::map<std::vector<uint32_t>, std::string> m_cachedConfigFiles;
    std::deque<std::vector<uint32_t>> m_cachedConfigFilesQueue;
    size_t m_cachedConfigFilesLimit = 10;
    TCPIPMessage m_genomeMessage = {};
    bool m_connected = false;

    bool m_debug = false;
};

#endif // OBJECTIVEMAINENET_H
