/*
 *  ObjectiveMainMPI.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "ObjectiveMainMPI.h"

ObjectiveMainMPI::ObjectiveMainMPI()
{

}

#if 0

#if defined(USE_MPI)
#include <mpi.h>
#endif

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cfloat>
#include <time.h>
#include <vector>
#include <map>
#include <algorithm>

#include <chrono>
#include <thread>


#if !defined(_WIN32) && !defined(WIN32)
#include <unistd.h>
#endif

#include "ode/ode.h"

#include "Simulation.h"
#include "DataFile.h"
#include "GSUtil.h"
#include "PGDMath.h"
#include "ObjectiveMain.h"
#include "XMLConverter.h"
#include "Reporter.h"
#include "DataTarget.h"
#include "Driver.h"
#include "Joint.h"
#include "Muscle.h"
#include "Body.h"
#include "Geom.h"
#include "MD5.h"

#ifdef USE_UDP
#include "UDP.h"
#include <netdb.h>
#endif

#ifdef USE_TCP
#include "TCP.h"
#include <netdb.h>
static int g_submitCount = 0;
#endif

// Simulation global
static Simulation *gSimulation = 0;

// run control
bool gFinishedFlag = true;

// usleep between read failure
int gSleepTime = 10000;

// default host settings
int gPort = 8086;
char gHost[256] = "localhost";

// command line arguments - remember to set default values for these in ParseArguments otherwise they are sticky
static const char *gHostlistFilenamePtr = "hosts.txt";
static char *gConfigFilenamePtr = 0;
static char *gScoreFilenamePtr = 0;
static char *gOutputKinematicsFilenamePtr = 0;
static char *gOutputModelStateFilenamePtr = 0;
static char *gOutputWarehouseFilenamePtr = 0;
static double gOutputModelStateAtTime = -1;
static char *gInputKinematicsFilenamePtr = 0;
static double gOutputModelStateAtCycle = -1;
static double gOutputModelStateAtWarehouseDistance = -1;
static char *gInputWarehouseFilenamePtr = 0;
static bool gMungeModelStateFlag = false;
static bool gMungeRotationFlag = false;
static bool gNewStylePositionOutputs = false;
static int gRedundancyPercent = 0;
static char *gModelConfigFile = 0;
static double gSimulationTimeLimit = -1;
static int gRunTimeLimit = 0;
static double gWarehouseFailDistanceAbort = 0;


static double gLastTime = 0;
static double gCurrentTime = 0;
static double gSimulationTime = 0;
static double gIOTime = 0;

static XMLConverter gXMLConverter;

static std::vector<std::string> gOutputList;

static unsigned int gMD5[4] = {};

// hostlist globals
struct Hosts
{
    char host[256];
    int port;
};
static std::vector<Hosts>gHosts;
static int gUseHost = 0;

#if defined(USE_UDP) || defined(USE_TCP)
static void ParseHostlistFile(void);
#endif

#if defined(USE_UDP)
UDP gUDP;
#endif

#ifdef USE_TCP
TCP gTCP;
#endif

#if defined(USE_MPI)
#include "MPIStuff.h"
static int gRunID;
extern int gMPI_Comm_size;
extern int gMPI_Comm_rank;
#endif

#if !defined(USE_MPI) && !defined(USE_GAUL) && !defined(USE_QT_AGA)
int main(int argc, char *argv[])
#else
int GaitSymMain(int argc, char *argv[])
#endif
{

    // start by parsing the command line arguments
    ParseArguments(argc - 1, &(argv[1]));

    if (gModelConfigFile)
    {
        gXMLConverter.LoadBaseXMLFile(gModelConfigFile);
    }

#if defined(USE_UDP) || defined(USE_TCP)
    ParseHostlistFile();
#endif

#if defined(USE_UDP)
    if ((gUDP.StartListener(0)) == -1)
    {
        std::cerr << "Error setting up listener\n";
        return 1;
    }
    if ((gUDP.StartTalker()) == -1)
    {
        std::cerr << "Error setting up talker\n";
        return 1;
    }
#endif


    // another never returned loop (exits are in WriteModel when required)
    gLastTime = GSUtil::GetTime();
    long runTime = 0;
    long startTime = time(0);
    unsigned int runCount = 0;
    while(gRunTimeLimit == 0 || runTime <= gRunTimeLimit)
    {
        runTime = time(0) - startTime;

        if (gFinishedFlag)
        {
            if (ReadModel() == 0)
            {
                gFinishedFlag = false;

                for (unsigned int i = 0; i < gOutputList.size(); i++)
                {
                    if (gSimulation->GetBodyList()->find(gOutputList[i]) != gSimulation->GetBodyList()->end()) (*gSimulation->GetBodyList())[gOutputList[i]]->setDump(true);
                    if (gSimulation->GetMuscleList()->find(gOutputList[i]) != gSimulation->GetMuscleList()->end()) (*gSimulation->GetMuscleList())[gOutputList[i]]->setDump(true);
                    if (gSimulation->GetGeomList()->find(gOutputList[i]) != gSimulation->GetGeomList()->end()) (*gSimulation->GetGeomList())[gOutputList[i]]->setDump(true);
                    if (gSimulation->GetJointList()->find(gOutputList[i]) != gSimulation->GetJointList()->end()) (*gSimulation->GetJointList())[gOutputList[i]]->setDump(true);
                    if (gSimulation->GetDriverList()->find(gOutputList[i]) != gSimulation->GetDriverList()->end()) (*gSimulation->GetDriverList())[gOutputList[i]]->setDump(true);
                    if (gSimulation->GetDataTargetList()->find(gOutputList[i]) != gSimulation->GetDataTargetList()->end()) (*gSimulation->GetDataTargetList())[gOutputList[i]]->setDump(true);
                    if (gSimulation->GetReporterList()->find(gOutputList[i]) != gSimulation->GetReporterList()->end()) (*gSimulation->GetReporterList())[gOutputList[i]]->setDump(true);
                }
            }
#ifndef USE_MPI
            else
            {
                std::this_thread::sleep_for(std::chrono::microseconds(gSleepTime));
            }
#endif
        }
        else
        {
            gCurrentTime = GSUtil::GetTime();
            gIOTime += (gCurrentTime - gLastTime);
            gLastTime = gCurrentTime;
            while (gSimulation->ShouldQuit() == false)
            {
                gSimulation->UpdateSimulation();

                if (gSimulation->TestForCatastrophy()) break;
            }
            gCurrentTime = GSUtil::GetTime();
            gSimulationTime += (gCurrentTime - gLastTime);
            gLastTime = gCurrentTime;

            gFinishedFlag = true;
            runCount++;
            if (runCount % 100 == 0) std::cerr << "gSimulationTime=" << gSimulationTime << " gIOTime=" << gIOTime << "\n" << std::flush;
            if (WriteModel()) return 0;
        }
    }
    return 0;
}


void ParseArguments(int argc, char ** argv)
{
    // command line arguments globals
    gHostlistFilenamePtr = "hosts.txt";
    gConfigFilenamePtr = 0;
    gScoreFilenamePtr = 0;
    gOutputKinematicsFilenamePtr = 0;
    gOutputModelStateFilenamePtr = 0;
    gOutputWarehouseFilenamePtr = 0;
    gOutputModelStateAtTime = -1;
    gInputKinematicsFilenamePtr = 0;
    gOutputModelStateAtCycle = -1;
    gOutputModelStateAtWarehouseDistance = -1;
    gInputWarehouseFilenamePtr = 0;
    gMungeModelStateFlag = false;
    gMungeRotationFlag = false;
    gNewStylePositionOutputs = false;
    gRedundancyPercent = 0;
    gModelConfigFile = 0;
    gSimulationTimeLimit = -1;
    gRunTimeLimit = 0;
    gWarehouseFailDistanceAbort = 0;

    int i;

    // do some simple stuff with command line arguments

    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--score") == 0 ||
            strcmp(argv[i], "-s") == 0)
        {
            i++;
            if (i >= argc)
            {
                std::cerr << "Error parsing score filename\n";
                exit(1);
            }
            gScoreFilenamePtr = argv[i];
        }
        else
            if (strcmp(argv[i], "--config") == 0 ||
                strcmp(argv[i], "-c") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing config filename\n";
                    exit(1);
                }
                gConfigFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--outputName") == 0 ||
                strcmp(argv[i], "-on") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing outputName\n";
                    exit(1);
                }
                gOutputList.push_back(std::string(argv[i]));
            }
        else
            if (strcmp(argv[i], "--hostList") == 0 ||
                strcmp(argv[i], "-L") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --hostList\n";
                    exit(1);
                }
                gHostlistFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--runTimeLimit") == 0 ||
                strcmp(argv[i], "-r") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --runTimeLimit\n";
                    exit(1);
                }
                gRunTimeLimit = strtol(argv[i], 0, 10);
            }
        else
            if (strcmp(argv[i], "--warehouseFailDistanceAbort") == 0 ||
                strcmp(argv[i], "-WF") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --warehouseFailDistanceAbort\n";
                    exit(1);
                }
                gWarehouseFailDistanceAbort = strtod(argv[i], 0);
            }
        else
            if (strcmp(argv[i], "--outputKinematics") == 0 ||
                strcmp(argv[i], "-K") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing output kinematics filename\n";
                    exit(1);
                }
                gOutputKinematicsFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--inputKinematics") == 0 ||
                strcmp(argv[i], "-J") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing input kinematics filename\n";
                    exit(1);
                }
                gInputKinematicsFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--outputWarehouse") == 0 ||
                strcmp(argv[i], "-H") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing output warehouse filename\n";
                    exit(1);
                }
                gOutputWarehouseFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--outputModelStateFile") == 0 ||
                strcmp(argv[i], "-M") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing model state filename\n";
                    exit(1);
                }
                gOutputModelStateFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--outputModelStateAtTime") == 0 ||
                strcmp(argv[i], "-t") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --outputModelStateAtTime\n";
                    exit(1);
                }
                gOutputModelStateAtTime = strtod(argv[i], 0);
            }
        else
            if (strcmp(argv[i], "--outputModelStateAtCycle") == 0 ||
                strcmp(argv[i], "-T") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --outputModelStateAtCycle\n";
                    exit(1);
                }
                gOutputModelStateAtCycle = strtod(argv[i], 0);
            }
        else
            if (strcmp(argv[i], "--outputModelStateAtWarehouseDistance") == 0 ||
                strcmp(argv[i], "-OW") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --outputModelStateAtWarehouseDistance";
                    exit(1);
                }
                gOutputModelStateAtWarehouseDistance = strtod(argv[i], 0);
            }
        else
            if (strcmp(argv[i], "--simulationTimeLimit") == 0 ||
                strcmp(argv[i], "-ST") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --simulationTimeLimit";
                    exit(1);
                }
                gSimulationTimeLimit = strtod(argv[i], 0);
        }

        else
            if (strcmp(argv[i], "--InputWarehouse") == 0 ||
                strcmp(argv[i], "-IW") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing input warehouse filename\n";
                    exit(1);
                }
                gInputWarehouseFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--redundancyPercent") == 0 ||
                strcmp(argv[i], "-R") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing --redundancyPercent\n";
                    exit(1);
                }
                gRedundancyPercent = strtol(argv[i], 0, 10);
            }
        else
            if (strcmp(argv[i], "--MungeModelState") == 0 ||
                strcmp(argv[i], "-U") == 0)
            {
                gMungeModelStateFlag = true;
            }
        else
            if (strcmp(argv[i], "--MungeRotation") == 0 ||
                strcmp(argv[i], "-u") == 0)
            {
                gMungeRotationFlag = true;
            }
        else
            if (strcmp(argv[i], "--NewStylePositionOutputs") == 0 ||
                strcmp(argv[i], "-O") == 0)
            {
                gNewStylePositionOutputs = true;
            }
        else
            if (strcmp(argv[i], "--ModelConfigFile") == 0 ||
                strcmp(argv[i], "-m") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing input ModelConfigFile filename\n";
                    exit(1);
                }
                gModelConfigFile = argv[i];
            }
        else
            if (strcmp(argv[i], "--Server") == 0 ||
                strcmp(argv[i], "-v") == 0)
            {
                i++;
                if (i >= argc)
                {
                    std::cerr << "Error parsing Server - host missing\n";
                    exit(1);
                }
                Hosts newHost;
                strncpy(newHost.host, argv[i], 255);
                char *colonPtr = strstr(newHost.host, ":");
                if (colonPtr == 0)
                {
                    std::cerr << "Error parsing Server - bad host name";
                    exit(1);
                }
                *colonPtr = 0;
                colonPtr++;
                newHost.port = strtol(colonPtr, 0, 10);
                gHosts.push_back(newHost);
            }
        else
            if (strcmp(argv[i], "--quiet") == 0 ||
                strcmp(argv[i], "-q") == 0)
            {
                freopen ("/dev/null", "w", stdout);
                freopen ("/dev/null", "w", stderr);
            }
        else
            if (strcmp(argv[i], "--help") == 0 ||
                strcmp(argv[i], "-h") == 0 ||
                strcmp(argv[i], "-?") == 0)
            {
                std::cerr << "\nObjective build " << __DATE__ << " " << __TIME__ << "\n\n";
                std::cerr << "-c filename, --config filename\n";
                std::cerr << "Reads filename rather than the default config.xml as the config data\n\n";
                std::cerr << "-s filename, --score filename\n";
                std::cerr << "Writes filename rather than the default score.tmp as the fitness data\n\n";
                std::cerr << "-L filename, --hostList filename\n";
                std::cerr << "Set the filename for the list of servers for the socket version\n\n";
                std::cerr << "-v ip.or.internet.name:port, --Server ip.or.internet.name:port";
                std::cerr << "Adds a specific server:port to the list of servers for the socket version\n\n";
                std::cerr << "-r n, --runTimeLimit n\n";
                std::cerr << "Quits the program (approximately) after it has run n seconds\n\n";
                std::cerr << "-ST x, --simulationTimeLimit n\n";
                std::cerr << "Quits the program after it has run the simulation for x seconds of simulation time\n\n";
                std::cerr << "-WF x, --warehouseFailDistanceAbort n\n";
                std::cerr << "Quits the program when the warehouse distance is greater than n (if n is negative, when less than -n\n\n";
                std::cerr << "-J filename, --inputKinematics filename\n";
                std::cerr << "Reads tab-delimited kinematic data from filename\n\n";
                std::cerr << "-K filename, --outputKinematics filename\n";
                std::cerr << "Writes tab-delimited kinematic data to filename\n\n";
                std::cerr << "-H filename, --outputWarehouse filename\n";
                std::cerr << "Writes tab-delimited gait warehouse data to filename\n\n";
                std::cerr << "-M filename, --outputModelStateFile filename\n";
                std::cerr << "Sets the model state filename\n\n";
                std::cerr << "-t x, --outputModelStateAtTime x\n";
                std::cerr << "Writes the model state to model state file at time x\n\n";
                std::cerr << "-T n, --outputModelStateAtCycle x\n";
                std::cerr << "Writes the model state to model state file at cycle x\n\n";
                std::cerr << "-OW n, --outputModelStateAtWarehouseDistance x\n";
                std::cerr << "Writes the model state to model state file when warehouse distance exceeds x\n\n";
                std::cerr << "-IW filename, --InputWarehouse filename\n";
                std::cerr << "Reads a tab-delimited gait warehouse file\n\n";
                std::cerr << "-R n, --redundancyPercent n\n";
                std::cerr << "% redundancy for forward error correction with UDP (set over 0 for effect)\n\n";
                std::cerr << "-U, --MungeModelState\n";
                std::cerr << "Munges the linear data in the model state file\n\n";
                std::cerr << "-u, --MungeRotation\n";
                std::cerr << "Munges the rotation data in the model state file\n\n";
                std::cerr << "-e, --ModelStateWorld\n";
                std::cerr << "Uses World data in the model state file\n\n";
                std::cerr << "-N, --NewStylePositionInputs\n";
                std::cerr << "Uses new standardised position and quaternion inputs\n\n";
                std::cerr << "-O, --NewStylePositionOutputs\n";
                std::cerr << "Uses new standardised position and quaternion outputs\n\n";
                std::cerr << "-m, --ModelConfigFile\n";
                std::cerr << "Use a model config file that can be substituted by an external genome\n\n";
                std::cerr << "-q, --quiet\n";
                std::cerr << "Suppresses stdout and stderr messages by redirecting to /dev/null\n\n";
                std::cerr << "-on, --outputName\n";
                std::cerr << "Outputs the dump file for the named object\n\n";
                std::cerr << "-h, -?, --help\n";
                std::cerr << "Prints this message!\n\n";
                exit(1);
            }
        else
        {
            std::cerr << "Unrecognised option. Try 'objective --help' for more info\n";
            exit(1);
        }
}

for (i = 0; i < argc; i++)
{
    if (strcmp(argv[i], "--outputName") == 0 || strcmp(argv[i], "-on") == 0)
    {
        i++;
        if (i >= argc)
        {
            std::cerr << "Error parsing debugNameFilter\n";
            exit(1);
        }
        gOutputList.push_back(std::string(argv[i]));
    }
}

}

// this routine attemps to read the model specification and initialise the simulation
// it returns zero on success
int ReadModel(void *userData)
{
    DataFile myFile;
#if !defined(USE_UDP) && !defined(USE_TCP) && !defined(USE_MPI)
    myFile.SetExitOnError(true);
#endif

    // load the config file
#if defined(USE_UDP)

    // get model config file from server

    try
    {
        struct hostent *he;
        struct sockaddr_in their_addr;
        if ((he = gethostbyname(gHosts[gUseHost].host)) == NULL) throw __LINE__;
        their_addr.sin_family = AF_INET; // host byte order
        their_addr.sin_port = htons(gHosts[gUseHost].port); // short, network byte order
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(their_addr.sin_zero), 0, 8); // zero the rest of the struct

        gUDP.BumpUDPPacketID();
        ((RequestSendGenomeUDPPacket *)gUDP.GetUDPPacket())->type = request_send_genome;
        ((RequestSendGenomeUDPPacket *)gUDP.GetUDPPacket())->port = gUDP.GetMyAddress()->sin_port;
        ((RequestSendGenomeUDPPacket *)gUDP.GetUDPPacket())->packetID = gUDP.GetUDPPacketID();
        int numBytes;
        if ((numBytes = gUDP.SendUDPPacket(&their_addr, sizeof(RequestSendGenomeUDPPacket))) == -1) throw __LINE__;

        if (gUDP.CheckReceiver(100000) != 1) throw __LINE__;

        char *buf;
        if (gRedundancyPercent <= 0)
        {
            if ((numBytes = gUDP.ReceiveText(&buf, gUDP.GetUDPPacketID())) == -1)  throw __LINE__;
        }
        else
        {
            if ((numBytes = gUDP.ReceiveFEC(&buf, gUDP.GetUDPPacketID(), gRedundancyPercent + 100)) == -1)  throw __LINE__;
        }

        ((GenomeReceivedUDPPacket *)gUDP.GetUDPPacket())->type = genome_received;
        ((GenomeReceivedUDPPacket *)gUDP.GetUDPPacket())->port = gUDP.GetMyAddress()->sin_port;
        ((GenomeReceivedUDPPacket *)gUDP.GetUDPPacket())->packetID = gUDP.GetUDPPacketID();
        if ((numBytes = gUDP.SendUDPPacket(&their_addr, sizeof(GenomeReceivedUDPPacket))) == -1) throw __LINE__;

        myFile.SetRawData(buf, strlen(buf));
        delete [] buf;

    }

    catch (int e)
    {
        gUseHost++;
        if (gUseHost >= gHosts.size()) gUseHost = 0;
        return 1;
    }

#elif defined(USE_TCP)


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
    TCPIPMessage *messagePtr = (TCPIPMessage *)buffer;

    try
    {
        status = gTCP.StartClient(gHosts[gUseHost].port, gHosts[gUseHost].host);
        if (status != 0) throw -1 * __LINE__;

        if (gXMLConverter.GetSmartSubstitutionFlag() == false)
        {
            strcpy(buffer, "req_xml_length");
            numBytes = gTCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
            //OUT_VAR(buffer);
            //OUT_VAR(numBytes);
            if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

            numBytes = gTCP.ReceiveData(buffer, TCPIPMessage::StandardMessageSize, 10, 0);
            //OUT_VAR(numBytes);
            if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;
            len = messagePtr->length;
            //OUT_VAR(len);
            char *xmlbuf = new char[len];

            strcpy(buffer, "req_xml_data");
            numBytes = gTCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
            //OUT_VAR(buffer);
            if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

            numBytes = gTCP.ReceiveData(xmlbuf, len, 10, 0);
            //OUT_VAR(numBytes);
            if (numBytes < len) throw __LINE__;
            memcpy(gMD5, md5(xmlbuf, len), sizeof(gMD5)); // the is the md5 score of everything that is sent (which includes a terminating zero)
            // std::cerr << hexDigest(gMD5) << "\n";

            gXMLConverter.LoadBaseXMLString(xmlbuf);
            delete [] xmlbuf;
        }

        strcpy(buffer, "req_send_length");
        numBytes = gTCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

        numBytes = gTCP.ReceiveData(buffer, TCPIPMessage::StandardMessageSize, 10, 0);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;
        len = messagePtr->length;
        g_submitCount = messagePtr->runID;
        // std::cerr << hexDigest((const unsigned int *)ptr) << "\n";
        if (memcmp(messagePtr->md5, gMD5, sizeof(gMD5)))
        {
            gXMLConverter.SetSmartSubstitutionFlag(false);
            throw __LINE__;
        }
        char *buf = new char[len];

        strcpy(buffer, "req_send_data");
        numBytes = gTCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;

        //OUT_VAR(len);
        //OUT_VAR(g_submitCount);

        numBytes = gTCP.ReceiveData(buf, len, 10, 0);
        if (numBytes < len) throw __LINE__;
        //OUT_VAR(buf);

        double *dPtr = (double *)buf;
        int genomeLength = len / sizeof(double);
        gXMLConverter.ApplyGenome(genomeLength, dPtr);
        myFile.SetRawData((char *)gXMLConverter.GetFormattedXML(&len), strlen((char *)gXMLConverter.GetFormattedXML(&len)));
        delete [] buf;
        gTCP.StopClient();
    }

    catch (int e)
    {
        if (e > 0) gTCP.StopClient();
        gUseHost++;
        if (gUseHost >= gHosts.size()) gUseHost = 0;
        return 1;
    }

#elif defined(USE_MPI)
    MPI_Status status;
    if (gConfigFilenamePtr)
    {
        if (gModelConfigFile == 0)
        {
            myFile.ReadFile(gConfigFilenamePtr);
        }
        else
        {
            DataFile genomeData;
            double val;
            int ival, genomeSize;
            genomeData.ReadFile(gConfigFilenamePtr);
            genomeData.ReadNext(&ival);
            genomeData.ReadNext(&genomeSize);
            double *data = new double[genomeSize];
            for (int i = 0; i < genomeSize; i++)
            {
                genomeData.ReadNext(&val);
                data[i] = val;
                genomeData.ReadNext(&val); genomeData.ReadNext(&val); genomeData.ReadNext(&val);
                if (ival == -2) genomeData.ReadNext(&val); // skip the extra parameter
            }
            gXMLConverter.ApplyGenome(genomeSize, data);
            int len;
            char *buf = (char *)gXMLConverter.GetFormattedXML(&len);
            myFile.SetRawData(buf);
            delete [] data;
        }
    }
    else
    {
        MPI_Probe(MPI_ANY_SOURCE,    /* source */
                  MPI_ANY_TAG,       /* filter by tag */
                  MPI_COMM_WORLD,    /* default communicator */
                  &status);        /* info about the received message */

        int count;
        MPI_Get_count(&status, MPI_BYTE, &count);
        char *data = new char[count];
        MPI_Recv(data,               /* message buffer */
                 count,              /* max number of data items */
                 MPI_BYTE,           /* of type BYTE */
                 0,                  /* receive from server */
                 MPI_ANY_TAG,        /* any type of message */
                 MPI_COMM_WORLD,     /* default communicator */
                 &status);           /* info about the received message */

        int *iPtr = (int *)data;
        if (iPtr[0] == MPI_MESSAGE_ID_RELOAD_MODELCONFIG) // force a model reload
        {
            gXMLConverter.LoadBaseXMLString(data + 2 * sizeof(int));
            delete [] data;
            return 1;
        }
        else if (iPtr[0] == MPI_MESSAGE_ID_SEND_GENOME_DATA)
        {
            gRunID = iPtr[1];
            int genomeLength = iPtr[2];
            double *dPtr = (double *)(&iPtr[3]);

            if (gXMLConverter.GetSmartSubstitutionFlag() == false)
            {
                myFile.SetRawData(data);
            }
            else
            {
                gXMLConverter.ApplyGenome(genomeLength, dPtr);
                int len;
                char *buf = (char *)gXMLConverter.GetFormattedXML(&len);
                myFile.SetRawData(buf);
            }
            delete [] data;
        }
        else if (iPtr[0] == MPI_MESSAGE_ID_SEND_TIMINGS)
        {
            char results[2 * sizeof(int) + 2 * sizeof(double)];
            int *iPtr = (int *)results;
            double *dPtr = (double *)(&iPtr[2]);
            iPtr[0] = MPI_MESSAGE_ID_SEND_TIMINGS;
            iPtr[1] = gRunID;
            dPtr[0] = gSimulationTime;
            dPtr[1] = gIOTime;
            MPI_Send(results,            /* message buffer */
                     sizeof(results),    /* 'len' data item */
                     MPI_BYTE,           /* data items are bytes */
                     0,                  /* destination */
                     0,                  /* user chosen message tag */
                     MPI_COMM_WORLD);    /* server */

            return 1;
        }
        else if (iPtr[0] == MPI_MESSAGE_ID_ABORT_CLIENTS)
        {
            std::cerr << "Client exit via MPI_MESSAGE_ID_ABORT_CLIENTS\n";
            MPI_Finalize();
            exit(0);
        }
        else
        {
            delete [] data;
            return 1;
        }
    }

#else
    if (gModelConfigFile == 0)
    {
        myFile.ReadFile(gConfigFilenamePtr);
    }
    else
    {
        DataFile genomeData;
        double val;
        int ival, genomeSize;
        genomeData.ReadFile(gConfigFilenamePtr);
        genomeData.ReadNext(&ival);
        genomeData.ReadNext(&genomeSize);
        double *data = new double[genomeSize];
        for (int i = 0; i < genomeSize; i++)
        {
            genomeData.ReadNext(&val);
            data[i] = val;
            genomeData.ReadNext(&val); genomeData.ReadNext(&val); genomeData.ReadNext(&val);
            if (ival == -2) genomeData.ReadNext(&val); // skip the extra parameter
        }
        gXMLConverter.ApplyGenome(genomeSize, data);
        int len;
        char *buf = (char *)gXMLConverter.GetFormattedXML(&len);
        myFile.SetRawData(buf, len);
        delete [] data;
    }

#endif

    // create the simulation object
    gSimulation = new Simulation();
//    if (gOutputKinematicsFilenamePtr) gSimulation->SetOutputKinematicsFile(gOutputKinematicsFilenamePtr);
//    if (gInputKinematicsFilenamePtr) gSimulation->SetInputKinematicsFile(gInputKinematicsFilenamePtr);
    if (gOutputWarehouseFilenamePtr) gSimulation->SetOutputWarehouseFile(gOutputWarehouseFilenamePtr);
    if (gOutputModelStateFilenamePtr) gSimulation->SetOutputModelStateFile(gOutputModelStateFilenamePtr);
    if (gOutputModelStateAtTime >= 0) gSimulation->SetOutputModelStateAtTime(gOutputModelStateAtTime);
    if (gOutputModelStateAtCycle >= 0) gSimulation->SetOutputModelStateAtCycle(gOutputModelStateAtCycle);
    if (gInputWarehouseFilenamePtr) gSimulation->AddWarehouse(gInputWarehouseFilenamePtr);
    if (gOutputModelStateAtWarehouseDistance >= 0) gSimulation->SetOutputModelStateAtWarehouseDistance(gOutputModelStateAtWarehouseDistance);
    if (gMungeModelStateFlag) gSimulation->SetMungeModelStateFlag(true);
    if (gMungeRotationFlag) gSimulation->SetMungeRotationFlag(true);

    if (gSimulation->LoadModel(myFile.GetRawData(), myFile.GetSize()))
    {
        delete gSimulation;
        gSimulation = 0;
        return 1;
    }

    // late initialisation options
    if (gSimulationTimeLimit >= 0) gSimulation->SetTimeLimit(gSimulationTimeLimit);
    if (gWarehouseFailDistanceAbort != 0) gSimulation->SetWarehouseFailDistanceAbort(gWarehouseFailDistanceAbort);

    return 0;
}

// returns 0 if continuing
// returns 1 if exit requested
int WriteModel()
{
    double score = gSimulation->CalculateInstantaneousFitness();
    // if (gSimulation->TestForCatastrophy())
    //  score -= 100000;

#ifdef USE_MPI
    int mpi_Comm_rank;
    int rc = MPI_Comm_rank(MPI_COMM_WORLD, &mpi_Comm_rank);
    std::cerr << "MPIRank: " << mpi_Comm_rank <<
                 " Simulation Time: " << gSimulation->GetTime() <<
                 " Steps: " << gSimulation->GetStepCount() <<
                 " Score: " << score <<
                 " Mechanical Energy: " << gSimulation->GetMechanicalEnergy() <<
                 " Metabolic Energy: " << gSimulation->GetMetabolicEnergy() <<
                 " CPUTimeSimulation: " << gSimulationTime <<
                 " CPUTimeIO: " << gIOTime << "\n";
#else
    std::cerr << "Simulation Time: " << gSimulation->GetTime() <<
                 " Steps: " << gSimulation->GetStepCount() <<
                 " Score: " << score <<
                 " Mechanical Energy: " << gSimulation->GetMechanicalEnergy() <<
                 " Metabolic Energy: " << gSimulation->GetMetabolicEnergy() <<
                 " CPUTimeSimulation: " << gSimulationTime <<
                 " CPUTimeIO: " << gIOTime << "\n";
#endif

#if defined(USE_UDP)
    try
    {
        struct hostent *he;
        struct sockaddr_in their_addr;
        if ((he = gethostbyname(gHosts[gUseHost].host)) == NULL) throw __LINE__;
        their_addr.sin_family = AF_INET; // host byte order
        their_addr.sin_port = htons(gHosts[gUseHost].port); // short, network byte order
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(their_addr.sin_zero), 0, 8); // zero the rest of the struct

        ((SendResultUDPPacket *)gUDP.GetUDPPacket())->type = send_result;
        ((SendResultUDPPacket *)gUDP.GetUDPPacket())->result = score;
        ((SendResultUDPPacket *)gUDP.GetUDPPacket())->port = gUDP.GetMyAddress()->sin_port;
        ((SendResultUDPPacket *)gUDP.GetUDPPacket())->packetID = gUDP.GetUDPPacketID();
        int numBytes;
        if ((numBytes = gUDP.SendUDPPacket(&their_addr, sizeof(SendResultUDPPacket))) == -1) throw __LINE__;
    }
    catch (int e)
    {
    }
#elif defined(USE_TCP)
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
    TCPIPMessage *messagePtr = (TCPIPMessage *)buffer;
    try
    {
        for (int i = 0; i < 10; i++)
        {
            status = gTCP.StartClient(gHosts[gUseHost].port, gHosts[gUseHost].host);
            if (status == 0) break;
            sleep(1);
        }
        if (status != 0) throw -1 * __LINE__;

        strcpy(messagePtr->text, "result");
        messagePtr->score = score;
        messagePtr->runID = g_submitCount;
        memcpy(messagePtr->md5, gMD5, sizeof(gMD5));
        numBytes = gTCP.SendData(buffer, TCPIPMessage::StandardMessageSize);
        if (numBytes != TCPIPMessage::StandardMessageSize) throw __LINE__;
        gTCP.StopClient();
    }

    catch (int e)
    {
        std::cerr << "Unable to write result back to host " << gHosts[gUseHost].host << " on port " << gHosts[gUseHost].port << "\n";
        if (e > 0) gTCP.StopClient();
        return 1;
    }

//    old version
//    try
//    {
//        char buffer[16];
//        double *dPtr = (double *)buffer;
//        *dPtr++ = score;
//        int *iPtr = (int *)dPtr;
//        *iPtr = g_submitCount;
//        int numBytes;
//        // send the data
//        memcpy(buffer, &doubleScore, sizeof(doubleScore));
//        numBytes = gTCP.SendData(buffer, 16);
//        if (numBytes != 16) throw __LINE__;
//   }
//    catch (int e)
//    {
//    }
//    gTCP.StopClient();

#elif defined(USE_MPI)
    if (gConfigFilenamePtr) // MPI version used in filename mode so exit
    {
        delete gSimulation;
        return(1);
    }

    char results[2 * sizeof(int) + sizeof(double)];
    int *iPtr = (int *)results;
    double *dPtr = (double *)(&iPtr[2]);
    iPtr[0] = MPI_MESSAGE_ID_SEND_RESULTS;
    iPtr[1] = gRunID;
    dPtr[0] = score;
    MPI_Send(results,            /* message buffer */
             sizeof(results),    /* 'len' data item */
             MPI_BYTE,           /* data items are bytes */
             0,                  /* destination */
             0,                  /* user chosen message tag */
             MPI_COMM_WORLD);    /* server */
#else
    if (gScoreFilenamePtr)
    {
        FILE *out;
        out = fopen(gScoreFilenamePtr, "wb");
        fwrite(&score, sizeof(double), 1, out);
        fclose(out);
    }
#endif

    delete gSimulation;

#if ! defined (USE_UDP) && ! defined (USE_TCP) && ! defined(USE_MPI)
    std::cerr << "exiting\n";
    return(1);
#else
    return(0);
#endif
}

bool GetOption(char ** begin, char ** end, const std::string &option, char **ptr)
{
    char **itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        *ptr = *itr;
        return true;
    }
    return false;
}

bool GetOption(char ** begin, char ** end, const std::string &option, double *ptr)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        *ptr = strtod(*itr, 0);
        return true;
    }
    return false;
}

bool GetOption(char ** begin, char ** end, const std::string &option, int *ptr)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        *ptr = strtol(*itr, 0, 10);
        return true;
    }
    return false;
}

bool GetOption(char** begin, char** end, const std::string &option)
{
    return std::find(begin, end, option) != end;
}

#if defined(USE_UDP) || defined(USE_TCP)
// read the file containing a list of hosts and ports
void ParseHostlistFile(void)
{
    char buffer[256];
    char *tokens[256];
    bool result;
    int count;
    Hosts host;
    DataFile hostListFile;
    hostListFile.SetExitOnError(false);

    if (hostListFile.ReadFile(gHostlistFilenamePtr))
    {
        if (gHosts.size() == 0)
        {
            strcpy(host.host, "localhost");
            host.port = 8086;
            gHosts.push_back(host);
            std::cerr << "No hosts file. Using localhost:8086\n";
        }
    }
    else
    {
        do
        {
            result = hostListFile.ReadNextLine(buffer, 256, true, '#');
            count = DataFile::ReturnTokens(buffer, tokens, 256);
            if (count >= 2)
            {
                strcpy(host.host, tokens[0]);
                host.port = strtol(tokens[1], 0, 10);
                gHosts.push_back(host);
            }
        } while (result == false);
    }

    if (gHosts.size() == 0)
    {
        std::cerr << "Could not get a list of hosts from " << gHostlistFilenamePtr << "\n";
        exit(-1);
    }
}

#endif

#endif
