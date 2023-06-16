/*
 *  UDP.cpp
 *  GeneralGA
 *
 *  Created by Bill Sellers on 30/06/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include "ThreadedUDP.h"


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <map>
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cstring>

#if defined(_WIN32) || defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


ThreadedUDP::ThreadedUDP()
{
    // note windows.h has macros for min & max which cause problems but putting std::max in brackets means that the STL version is used
    m_biggestPacket = (std::max)({sizeof(udp::SendTextUDPPacket), sizeof(udp::SendFECUDPPacket), sizeof(udp::RequestItemUDPPacket)});
    m_smallestPacket = (std::min)({sizeof(udp::SendTextUDPPacket), sizeof(udp::SendFECUDPPacket), sizeof(udp::RequestItemUDPPacket)});
}

ThreadedUDP::~ThreadedUDP()
{
    m_abortReceiveThread = true;
    m_abortSendThread = true;
    if (m_receiveThread.joinable()) m_receiveThread.join();
    if (m_sendThread.joinable()) m_sendThread.join();
}

void ThreadedUDP::ReceiveThread()
{
    while (!m_abortReceiveThread)
    {
        std::shared_ptr<Packet> packet = std::make_shared<Packet>();
        packet->data.resize(m_biggestPacket);

        socklen_t addrLen = sizeof(packet->address);
        int numBytes = recvfrom(m_RecSockfd, packet->data.data(), int(m_biggestPacket), 0, reinterpret_cast<struct sockaddr *>(&packet->address), &addrLen);
        if (numBytes < int(m_smallestPacket)) continue;
        packet->data.resize((std::min)(numBytes, int(m_biggestPacket)));
        packet->used = false;
        uint64_t index = uint64_t(packet->address.sin_addr.s_addr) * packet->address.sin_port;
        std::unique_lock<std::mutex> lock(m_receiveMutex);
        m_receivePacketQueue.push_back(packet);
        while (m_receivePacketQueue.size() > m_maxReceiveQueueSize)
        {
            m_receivePacketQueue.front()->used = true;
            m_receivePacketQueue.pop_front();
        }

        if (!m_useSimpleReceiveQueue)
        {
            // and this is the clever bit where we create separate queues for each sender
            auto it = m_receivePacketMap.find(index);
            if (it == m_receivePacketMap.end()) { m_receivePacketMap[index] = std::deque<std::shared_ptr<Packet>>({packet}); }
            else
            {
                it->second.push_back(packet);
                while (it->second.size() > m_maxReceiveQueueSize)
                {
                    it->second.front()->used = true;
                    it->second.pop_front();
                }
            }
        }
    }
}

std::shared_ptr<ThreadedUDP::Packet> ThreadedUDP::GetNextPacket()
{
    std::shared_ptr<Packet> packet;
    std::unique_lock<std::mutex> lock(m_receiveMutex);
    if (m_receivePacketQueue.size() == 0) return packet;
    while (true)
    {
        packet = m_receivePacketQueue.front();
        m_receivePacketQueue.pop_front();
        if (m_receivePacketQueue.size() == 0)
        {
            if (packet->used == true) return std::shared_ptr<Packet>();
            break;
        }
        if (packet->used == false) break;
    }
    packet->used = true;

    if (!m_useSimpleReceiveQueue)
    {
        // and this is the part where we always clean up the address based queues (because it should be quick)
        uint64_t index = uint64_t(packet->address.sin_addr.s_addr) * packet->address.sin_port;
        auto it = m_receivePacketMap.find(index);
        if (it != m_receivePacketMap.end())
        {
            for (auto it2 = it->second.begin(); it2 < it->second.end(); it++)
            {
                if (*it2 == packet)
                {
                    it->second.erase(it2);
                    break;
                }
            }
        }
    }
    return packet;
}

std::shared_ptr<ThreadedUDP::Packet> ThreadedUDP::GetNextPacket(uint64_t index)
{
    std::shared_ptr<Packet> packet;
    if (m_useSimpleReceiveQueue) return packet;
    std::unique_lock<std::mutex> lock(m_receiveMutex);
    auto it = m_receivePacketMap.find(index);
    if (it == m_receivePacketMap.end()) return packet;
    if (it->second.size() == 0) return packet;
    while (true)
    {
        packet = it->second.front();
        it->second.pop_front();
        if (packet->used == false) break;
        if (it->second.size() == 0) return std::shared_ptr<Packet>();
    }
    packet->used = true;
    // but we do not automatically clean up m_receivePacketQueue because it could be very slow
    // instead we discard used packets when they turn up in regular queue access
    return packet;
}

std::shared_ptr<ThreadedUDP::Packet> ThreadedUDP::GetNextPacket(const struct sockaddr_in &address)
{
    return GetNextPacket(uint64_t(address.sin_addr.s_addr) * address.sin_port);
}

std::shared_ptr<ThreadedUDP::Packet> ThreadedUDP::GetNextPacket(uint32_t ip4Address, uint16_t port)
{
    return GetNextPacket(uint64_t(ip4Address) * port);
}

void ThreadedUDP::ClearReceiveBuffers()
{
    std::unique_lock<std::mutex> lock(m_receiveMutex);
    m_receivePacketQueue.clear();
    m_receivePacketMap.clear();
}

void ThreadedUDP::RemoveUsedPackets()
{
    std::unique_lock<std::mutex> lock(m_receiveMutex);
    for (auto it = m_receivePacketQueue.begin(); it != m_receivePacketQueue.end();)
    {
        if (it->get()->used) { it = m_receivePacketQueue.erase(it); }
        else { it++; }
    }
    for (auto it = m_receivePacketMap.begin(); it != m_receivePacketMap.end();)
    {
        for (auto it2 = it->second.begin(); it2 != it->second.end();)
        {
            if (it2->get()->used) { it2 = it->second.erase(it2); }
            else { it2++; }
        }
    }
}

void ThreadedUDP::RemoveEmptyQueues()
{
    std::unique_lock<std::mutex> lock(m_receiveMutex);
    for (auto it = m_receivePacketMap.begin(); it != m_receivePacketMap.end();)
    {
        if (it->second.size() == 0) { it = m_receivePacketMap.erase(it); }
        else { it++; }
    }
}

void ThreadedUDP::ClearSendBuffer()
{
    std::unique_lock<std::mutex> lock(m_receiveMutex);
    m_sendPacketQueue.clear();
}

void ThreadedUDP::SendThread()
{
    while (!m_abortSendThread)
    {
        std::unique_lock<std::mutex> lock(m_sendMutex);
        if (m_sendPacketQueue.size() == 0)
        {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(uint64_t(m_usSleepEmptyQueue)));
            continue;
        }
        std::shared_ptr<Packet> packet = m_sendPacketQueue.front();
        int numBytes = sendto(m_RecSockfd, packet->data.data(), int(packet->data.size()), 0,
                              reinterpret_cast<const struct sockaddr *>(&packet->address), sizeof(packet->address));
        if (numBytes != int(packet->data.size())) // this is an error condition
        {
            if (m_debug) std::cerr << "Error sending packet to " << packet->address.sin_addr.s_addr << ":" << ntohs(packet->address.sin_port) << "\n";
        }
        m_sendPacketQueue.pop_front();
    }
}

bool ThreadedUDP::useSimpleReceiveQueue() const
{
    return m_useSimpleReceiveQueue;
}

void ThreadedUDP::setUseSimpleReceiveQueue(bool useSimpleReceiveQueue)
{
    if (useSimpleReceiveQueue && !m_useSimpleReceiveQueue)
    {
        std::unique_lock<std::mutex> lock(m_receiveMutex);
        m_receivePacketMap.clear();
    }
    m_useSimpleReceiveQueue = useSimpleReceiveQueue;
}

size_t ThreadedUDP::maxSendQueueSize() const
{
    return m_maxSendQueueSize;
}

void ThreadedUDP::setMaxSendQueueSize(size_t maxSendQueueSize)
{
    m_maxSendQueueSize = maxSendQueueSize;
}

size_t ThreadedUDP::maxReceiveQueueSize() const
{
    return m_maxReceiveQueueSize;
}

void ThreadedUDP::setMaxReceiveQueueSize(size_t maxReceiveQueueSize)
{
    m_maxReceiveQueueSize = maxReceiveQueueSize;
}

void ThreadedUDP::QueueSendPacket(std::shared_ptr<ThreadedUDP::Packet> packet)
{
    std::unique_lock<std::mutex> lock(m_sendMutex);
    m_sendPacketQueue.push_back(packet);
    while (m_sendPacketQueue.size() > m_maxSendQueueSize) m_sendPacketQueue.pop_front();
}

int ThreadedUDP::ReceiveUDPPacket(udp::UDPPacket *packet, int numBytes, struct sockaddr_in *sender)
{
    std::shared_ptr<Packet> queuedPacket = GetNextPacket();
    if (queuedPacket == nullptr) return -1;
    size_t bytesToCopy = (std::min)(size_t(numBytes), queuedPacket->data.size());
    std::memcpy(packet, queuedPacket->data.data(), bytesToCopy);
    *sender = queuedPacket->address;
    return int(bytesToCopy);
}

int ThreadedUDP::SendUDPPacket(const struct sockaddr_in &destination, const udp::UDPPacket *packet, int numBytes)
{
    std::shared_ptr<Packet> queuedPacket = std::make_shared<Packet>();
    queuedPacket->address = destination;
    queuedPacket->data.assign(reinterpret_cast<const char *>(packet), reinterpret_cast<const char *>(packet) + numBytes);
    QueueSendPacket(queuedPacket);
    return numBytes;
}

int ThreadedUDP::StartListener(uint16_t port)
{
    int status = UDP::StartListener(port);
    if (status) return status;
    m_receiveThread = std::thread(&ThreadedUDP::ReceiveThread, this);
    m_sendThread = std::thread(&ThreadedUDP::SendThread, this);
    return 0;
}

void ThreadedUDP::StopListener()
{
    m_abortReceiveThread = true;
    if (m_receiveThread.joinable()) m_receiveThread.join();
    if (m_sendThread.joinable()) m_sendThread.join();
    UDP::StopListener();
}

int ThreadedUDP::CheckReceiver(uint64_t usecTimeout)
{
    // 0 if nothing ready, 1 if something there, -1 on error
    std::unique_lock<std::mutex> lock(m_receiveMutex);
    if (m_receivePacketQueue.size()) return 1;
    lock.unlock();
    uint64_t usCounter = 0;
    while (usCounter < usecTimeout)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(m_usCheckReceiverSleep));
        lock.lock();
        if (m_receivePacketQueue.size()) return 1;
        lock.unlock();
    }
    return 0;
}
