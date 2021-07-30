/*
 *  UDP.h
 *  GeneralGA
 *
 *  Created by Bill Sellers on 30/06/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include "UDP.h"

#include <random>
#include <mutex>
#include <atomic>
#include <map>
#include <deque>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

class ThreadedUDP : public UDP
{
public:
    ThreadedUDP();
    ~ThreadedUDP();

    virtual int ReceiveUDPPacket(udp::UDPPacket *packet, int numBytes, struct sockaddr_in *sender);
    virtual int SendUDPPacket(const struct sockaddr_in &destination, const udp::UDPPacket *packet, int numBytes);
    virtual int StartListener(uint16_t port);
    virtual void StopListener();
//    virtual int StartTalker();
//    virtual void StopTalker();
    virtual int CheckReceiver(uint64_t usecTimeout);

    struct Packet
    {
        struct sockaddr_in address;
        std::vector<char> data;
        bool used = false;
    };

    std::shared_ptr<Packet> GetNextPacket();
    std::shared_ptr<Packet> GetNextPacket(uint64_t index);
    std::shared_ptr<Packet> GetNextPacket(const struct sockaddr_in &address);
    std::shared_ptr<Packet> GetNextPacket(uint32_t ip4Address, uint16_t port);

    void QueueSendPacket(std::shared_ptr<Packet> packet);

    void ClearReceiveBuffers();
    void RemoveUsedPackets();
    void RemoveEmptyQueues();

    void ClearSendBuffer();

    size_t maxReceiveQueueSize() const;
    void setMaxReceiveQueueSize(size_t maxReceiveQueueSize);

    size_t maxSendQueueSize() const;
    void setMaxSendQueueSize(size_t maxSendQueueSize);

    bool useSimpleReceiveQueue() const;
    void setUseSimpleReceiveQueue(bool useSimpleReceiveQueue);

private:
    void ReceiveThread();
    void SendThread();

    std::mutex m_receiveMutex;
    std::thread m_receiveThread;
    std::atomic<bool> m_abortReceiveThread = {false};
    std::deque<std::shared_ptr<Packet>> m_receivePacketQueue;
    std::map<uint64_t, std::deque<std::shared_ptr<Packet>>> m_receivePacketMap;
    std::atomic<size_t> m_maxReceiveQueueSize = {100};
    std::atomic<bool> m_useSimpleReceiveQueue = {true};

    std::mutex m_sendMutex;
    std::thread m_sendThread;
    std::atomic<bool> m_abortSendThread = {false};
    std::deque<std::shared_ptr<Packet>> m_sendPacketQueue;
    uint64_t m_usSleepEmptyQueue = 10000;
    std::atomic<size_t> m_maxSendQueueSize = {1000};
    uint64_t m_usCheckReceiverSleep = 1000;

    size_t m_biggestPacket = 0;
    size_t m_smallestPacket = 0;
};

