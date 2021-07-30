/*
 *  UDP.h
 *  GeneralGA
 *
 *  Created by Bill Sellers on 30/06/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include <random>
#include <mutex>
#include <atomic>
#include <map>
#include <deque>
#include <string>
#include <vector>
#include <memory>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

namespace udp
{
enum UDPPacketType : uint32_t { send_text = 1, send_fec, request_item, send_score };
enum RequestItems : uint32_t { genome = 1, xml, score };

// ethernet MTU is 1500
// IP4 header is 20 bytes
// TCP header is 20 bytes (and can be up to 60 apparently)
// UDP header is 8 bytes

static const int kUDPPacketTextSize = 1500 - 20 - 20; // depending on packing the packets are 44 bytes plus the payload

struct UDPPacket
{
    UDPPacketType type = {};
};

struct SendTextUDPPacket : UDPPacket
{
    SendTextUDPPacket() { type = send_text; }
    uint64_t packetID = 0;
    uint64_t totalLength = 0;
    uint64_t numUDPPackets = 0;
    uint64_t packetCount = 0;
    uint64_t lenThisUDPPacket = 0;
    char text[kUDPPacketTextSize];
};

struct SendFECUDPPacket : UDPPacket
{
    SendFECUDPPacket() { type = send_fec; }
    uint64_t packetID = 0;
    uint64_t totalLength = 0;
    uint64_t numUDPPackets = 0;
    uint64_t packetCount = 0;
    uint64_t lenThisUDPPacket = 0;
    char text[kUDPPacketTextSize];
};

struct RequestItemUDPPacket : UDPPacket
{
    RequestItemUDPPacket() { type = request_item; }
    uint64_t packetID = 0;
    RequestItems item = {};
    uint32_t ip4Address = 0;
    uint32_t port = 0;
    uint32_t runID = 0;
    uint32_t redundancy = 0;
    double score = 0;
};

}

class UDP
{
public:
    UDP();
    virtual ~UDP();

    int ReceiveText(uint64_t matchID, std::string *text, struct sockaddr_in *sender);
    int SendText(uint64_t matchID, const struct sockaddr_in &destination, const std::string &text);
    int ReceiveFEC(uint64_t matchID, uint32_t percentRedundancy, std::string *data, struct sockaddr_in *sender);
    int SendFEC(uint64_t matchID, const struct sockaddr_in &destination, const std::string &data, uint32_t percentRedundancy);

    virtual int ReceiveUDPPacket(udp::UDPPacket *packet, int numBytes, struct sockaddr_in *sender);
    virtual int SendUDPPacket(const struct sockaddr_in &destination, const udp::UDPPacket *packet, int numBytes);
    virtual int StartListener(uint16_t port);
    virtual void StopListener();
//    virtual int StartTalker();
//    virtual void StopTalker();
    virtual int CheckReceiver(uint64_t usecTimeout);

    struct sockaddr_in *GetMyAddress() { return &m_myAddress; }

    void setDebug(bool debug);

protected:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    SOCKET m_RecSockfd = 0;
//    SOCKET m_SendSockfd = 0;
#else
    int m_RecSockfd = 0;
//    int m_SendSockfd = 0;
#endif
    bool m_debug = false;

private:
    struct sockaddr_in m_myAddress = {}; // my address information
    uint64_t m_packetID = 0;

    uint64_t m_receiveTimeout = 1000000; // microseconds
    uint64_t m_sendTimeout = 1000000; // microseconds

    std::mt19937_64 m_randomNumberGenerator;
    std::uniform_int_distribution<uint64_t> m_uint64Distribution;
};


class UDPUpDown
{
public:
   UDPUpDown();
   ~UDPUpDown();

   int status() const;

private:
   int m_status = 0;
};

class StopListenerGuard
{
public:
    StopListenerGuard(UDP *udp) { m_udp = udp; };
    ~StopListenerGuard() { m_udp->StopListener(); };
private:
    UDP *m_udp;
};

//class StopTalkerGuard
//{
//public:
//    StopTalkerGuard(UDP *udp) { m_udp = udp; };
//    ~StopTalkerGuard() { m_udp->StopTalker(); };
//private:
//    UDP *m_udp;
//};

