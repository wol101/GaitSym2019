/*
 *  UDP.h
 *  GeneralGA
 *
 *  Created by Bill Sellers on 30/06/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include <random>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

struct UDPRunSpecifier
{
    double startTime;
    uint64_t packetID;
    struct sockaddr_in runningOn;
    char *XMLdata;
    int XMLdataIndex;
    int XMLdataLen;
    int packetNumber;
};

enum UDPPacketType
{
    idle = 0,
    request_send_xml = 1,
    request_send_genome = 2,
    send_result = 3,
    send_text = 4,
    genome_received = 5,
    send_fec = 6
};

const int kUDPPacketTextSize = 1400; // the total sizeof(TextUDPPacket) needs to be < MTU - 40

struct UDPPacket
{
    UDPPacketType type;
    unsigned long packetID;
};

struct RequestSendGenomeUDPPacket: public UDPPacket
{
    int port;
    int index;
    int packetNumber;
};

struct SendResultUDPPacket: public UDPPacket
{
    int port;
    int index;
    double result;
};

struct SendTextUDPPacket: public UDPPacket
{
    int port;
    int index;
    int totalLength;
    int numUDPPackets;
    int packetCount;
    int lenThisUDPPacket;
    char text[kUDPPacketTextSize];
};

struct SendFECUDPPacket: public UDPPacket
{
    int port;
    int index;
    int totalLength;
    int numUDPPackets;
    int packetCount;
    int lenThisUDPPacket;
    char text[kUDPPacketTextSize];
};

struct GenomeReceivedUDPPacket: public UDPPacket
{
    int port;
    int index;
};

class UDP
{
public:
    UDP();
    virtual ~UDP();

    int StartListener(int MYPORT);
    void StopListener();
    int StartTalker();
    void StopTalker();
    int ReceiveUDPPacket(int numBytes);
    int SendUDPPacket(struct sockaddr_in *destination, int numBytes);
    int ReceiveText(char **buf, unsigned long matchID);
    int SendText(struct sockaddr_in *destination, int index, char *text, int len);
    int ReceiveFEC(char **buf, unsigned long matchID, int percentRedundancy);
    int SendFEC(struct sockaddr_in *destination, int index, char *text, int len, int percentRedundancy);
    int CheckAlive(struct sockaddr_in * /* destination */) { return 0; }
    int CheckReceiver(long usecTimeout);

    uint64_t BumpUDPPacketID() { m_packetID++; if (m_packetID == 0) m_packetID++; return m_packetID; }
    uint64_t GetUDPPacketID() { return m_packetID; }

    UDPPacket *GetUDPPacket() { return &m_packet; }
    struct sockaddr_in  *GetMyAddress() { return &m_myAddress; }
    struct sockaddr_in  *GetSenderAddress() { return &m_senderAddress; }

private:

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    SOCKET m_RecSockfd = 0;
    SOCKET m_SendSockfd = 0;
#else
    int m_RecSockfd = 0;
    int m_SendSockfd = 0;
#endif
    struct sockaddr_in m_myAddress; // my address information
    struct sockaddr_in m_senderAddress; // my address information
    UDPPacket m_packet;
    uint64_t m_packetID = 0;

    std::mt19937_64 m_randomNumberGenerator;
    std::uniform_int_distribution<uint64_t> m_uint64Distribution;
};


