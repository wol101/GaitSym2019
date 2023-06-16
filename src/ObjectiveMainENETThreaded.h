/*
 *  ObjectiveMainENETThreaded.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef OBJECTIVEMAINENETTHREADED_H
#define OBJECTIVEMAINENETTHREADED_H

#include "XMLConverter.h"
#include "ArgParse.h"
#include "TCPIPMessage.h"

#include "enet/enet.h"

#include <string>
#include <vector>
#include <map>
#include <deque>
#include <memory>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>

class Simulation;

class ThreadedComms
{
public:
    ThreadedComms();
    ~ThreadedComms();

    struct Host
    {
    public:
        std::string host;
        int port;
        ENetAddress enetAddress;
    };

    struct ReceivePackage
    {
        TCPIPMessage message;
        std::string content;
        Host host;
    };

    struct SendPackage
    {
        TCPIPMessage message;
        size_t retryCount = 0;
        Host host;
    };

    int Initialise(const std::vector<Host> &hosts);

    ReceivePackage UnqueueReceivePackage();
    void QueueSendPackage(const SendPackage &sendPackage);

    void setDebug(bool debug);

private:
    void SendThread();
    void ReceiveThread();
    std::string ReadModel();
    int WriteOutput();

    std::mutex m_mutex;
    std::thread m_sendThread;
    std::thread m_receiveThread;
    double m_receivePause = 1000; // milliseconds
    double m_receivePauseMultiplier = 1;
    double m_receivePauseMultiplierMax = 256;
    double m_receivePauseMultiplierInflate = 1.5;
    double m_sendPause = 500; // milliseconds
    double m_sendPauseMultiplier = 1;
    double m_sendPauseMultiplierMax = 256;
    double m_sendPauseMultiplierInflate = 1.5;
    std::atomic<bool> abortThread = {false};

    std::deque<ReceivePackage> m_receiveQueue;
    size_t m_receiveQueueMaxSize = 1;
    std::deque<SendPackage> m_sendQueue;

    XMLConverter m_XMLConverter;
    std::vector<Host> m_hosts;
    Host m_outputHost = {};
    size_t m_currentHost = 0;
    ENetHost *m_client = nullptr;
    ENetPeer *m_peer = nullptr;
    unsigned int m_MD5[4] = {};
    int m_timeout = 100000; // milliseconds
    std::map<std::vector<uint32_t>, std::string> m_cachedConfigFiles;
    std::deque<std::vector<uint32_t>> m_cachedConfigFilesQueue;
    size_t m_cachedConfigFilesLimit = 10;
    TCPIPMessage m_genomeMessage = {};
    bool m_connected = false;
    bool m_debug = false;
    bool m_enetInitialised = false;

    std::mt19937_64 m_genReceive;
    std::uniform_real_distribution<double> m_distribReceive;
    std::mt19937_64 m_genSend;
    std::uniform_real_distribution<double> m_distribSend;
};

class ObjectiveMainENETThreaded
{
public:
    ObjectiveMainENETThreaded(int argc, const char **argv);

    int Run();

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
    double m_simulationTimeLimit = -1;
    double m_warehouseFailDistanceAbort = 0;

    std::string m_configFilename;
    std::string m_outputWarehouseFilename;
    std::string m_outputModelStateFilename;
    std::string m_inputWarehouseFilename;
    std::string m_scoreFilename;

    XMLConverter m_XMLConverter;
    ArgParse m_argparse;

    std::vector<ThreadedComms::Host> m_hosts;
    ThreadedComms::ReceivePackage m_genomePackage = {};
    int m_sleepTime = 10000; // milliseconds
    std::mt19937_64 m_gen;
    std::uniform_real_distribution<double> m_distrib;

    bool m_debug = false;
    bool m_verbose = false;
};


#endif // OBJECTIVEMAINENETTHREADED_H
