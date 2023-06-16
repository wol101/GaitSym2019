/*
 *  UDP.cpp
 *  GeneralGA
 *
 *  Created by Bill Sellers on 30/06/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include "UDP.h"

#include "FEC.h"

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
#include <assert.h>

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

UDP::UDP()
{
#ifdef NON_THREAD_SAFE_OK
    init_fec();
#endif
    std::random_device randomDevice;
    m_randomNumberGenerator.seed(randomDevice());
}

UDP::~UDP()
{
#if defined(_WIN32) || defined(WIN32)
    if (m_RecSockfd >= 0) closesocket(m_RecSockfd);
//    if (m_SendSockfd >= 0) closesocket(m_SendSockfd);
#else
    if (m_RecSockfd >= 0) close(m_RecSockfd);
//    if (m_SendSockfd >= 0) close(m_SendSockfd);
#endif
}

int UDP::StartListener(uint16_t port)
{
#if defined(_WIN32) || defined(WIN32)
    if ((m_RecSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return -1;
#else
    if ((m_RecSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) return -1;
#endif

    struct timeval tv;
    tv.tv_sec = long(m_receiveTimeout / 1000000);
    tv.tv_usec = long(m_receiveTimeout % 1000000);
    setsockopt(m_RecSockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

    tv.tv_sec = long(m_sendTimeout / 1000000);
    tv.tv_usec = long(m_sendTimeout % 1000000);
    setsockopt(m_RecSockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));

    m_myAddress = {};
    m_myAddress.sin_family = AF_INET; // host byte order
    m_myAddress.sin_port = htons(port); // short, network byte order
    m_myAddress.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP

    if (port != 0)
    {
        if (bind(m_RecSockfd, (struct sockaddr *)&m_myAddress, sizeof(struct sockaddr)) == -1)
        {
#if defined(_WIN32) || defined(WIN32)
            closesocket(m_RecSockfd);
#else
            close(m_RecSockfd);
#endif
            return -1;
        }
    }
    else
    {
        port = 0x7fff;
        while (true)
        {
            port++;
            m_myAddress.sin_port = htons(port); // short, network byte order
            if (bind(m_RecSockfd, (struct sockaddr *)&m_myAddress, sizeof(struct sockaddr)) == 0) break;
            if (port < 0x8000) // means that the uint16_t has wrapped around
            {
#if defined(_WIN32) || defined(WIN32)
                closesocket(m_RecSockfd);
#else
                close(m_RecSockfd);
#endif
                return -1;
            }
        }
    }

    m_packetID = m_uint64Distribution(m_randomNumberGenerator);
    if (m_packetID == 0) m_packetID++;

    if (m_debug) { std::cerr << "UDP::StartListener listening on port " << port << "\n"; }
    return 0;
}

void UDP::StopListener()
{
#if defined(_WIN32) || defined(WIN32)
    closesocket(m_RecSockfd);
#else
    close(m_RecSockfd);
#endif
    m_RecSockfd = 0;
    if (m_debug) { std::cerr << "UDP::StopListener\n"; }
    return;
}

//int UDP::StartTalker()
//{
//#if defined(_WIN32) || defined(WIN32)
//    if ((m_SendSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return -1;
//#else
//    if ((m_SendSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) return -1;
//#endif
//    struct timeval tv;
//    tv.tv_sec = long(m_sendTimeout / 1000000);
//    tv.tv_usec = long(m_sendTimeout % 1000000);
//    setsockopt(m_SendSockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));

//    if (m_debug) { std::cerr << "UDP::StartTalker\n"; }
//    return 0;
//}

//void UDP::StopTalker()
//{
//#if defined(_WIN32) || defined(WIN32)
//    closesocket(m_SendSockfd);
//#else
//    close(m_SendSockfd);
//#endif
//    m_SendSockfd = 0;
//    if (m_debug) { std::cerr << "UDP::StopTalker\n"; }
//    return;
//}

int UDP::ReceiveUDPPacket(udp::UDPPacket *packet, int numBytes, struct sockaddr_in *sender)
{
    // sockaddr_in starts with sockaddr so the cast to (struct sockaddr *) always works
    socklen_t addr_len = sizeof(struct sockaddr_in);
    numBytes = recvfrom(m_RecSockfd, reinterpret_cast<char *>(packet), numBytes, 0, reinterpret_cast<struct sockaddr *>(sender), &addr_len);
    return numBytes;
}

int UDP::SendUDPPacket(const struct sockaddr_in &destination, const udp::UDPPacket *packet, int numBytes)
{
    // sockaddr_in starts with sockaddr so the cast to (struct sockaddr *) always works
//    numBytes = sendto(m_SendSockfd, reinterpret_cast<const char *>(packet), numBytes, 0, reinterpret_cast<const struct sockaddr *>(&destination), sizeof(struct sockaddr_in));
    numBytes = sendto(m_RecSockfd, reinterpret_cast<const char *>(packet), numBytes, 0, reinterpret_cast<const struct sockaddr *>(&destination), sizeof(struct sockaddr_in));
    return numBytes;
}

int UDP::SendText(uint64_t matchID, const struct sockaddr_in &destination, const std::string &text)
{
    size_t p1 = 0;
    size_t bytesToSend = text.size();
    udp::SendTextUDPPacket packet;
    packet.packetID =matchID;
    packet.totalLength = text.size();
    packet.numUDPPackets = packet.totalLength / udp::kUDPPacketTextSize;
    if (packet.totalLength % udp::kUDPPacketTextSize) packet.numUDPPackets++;
    packet.packetCount = 0;

    while (bytesToSend)
    {
        if (bytesToSend >= udp::kUDPPacketTextSize) packet.lenThisUDPPacket = udp::kUDPPacketTextSize;
        else packet.lenThisUDPPacket = bytesToSend;
        std::memcpy(packet.text, &text[p1], packet.lenThisUDPPacket);
        if (m_debug) { std::cerr << "UDP::SendText Sending " << packet.lenThisUDPPacket  << "\n"; }
        if (SendUDPPacket(destination, &packet, sizeof(udp::SendTextUDPPacket)) == -1)
        {
            if (m_debug) { std::cerr << "UDP::SendText error in SendUDPPacket\n"; }
            return -1;
        }
        p1 += packet.lenThisUDPPacket;
        bytesToSend -= packet.lenThisUDPPacket;
        packet.packetCount++;
    }
    return int(packet.totalLength);
}

int UDP::ReceiveText(uint64_t matchID, std::string *text, struct sockaddr_in *sender)
{
    std::map<size_t, udp::SendTextUDPPacket>packetList;
    udp::SendTextUDPPacket packet;
    size_t len = 0;

    while (CheckReceiver(1000000) == 1)
    {
        if (ReceiveUDPPacket(&packet, sizeof(udp::SendTextUDPPacket), sender) == -1)
        {
            if (m_debug) { std::cerr << "UDP::ReceiveText error in ReceiveUDPPacket\n"; }
            return -1;
        }
        if (matchID == 0 || packet.packetID == matchID) // if matchID is set ignore packets with the wrong ID
        {
            if (packet.type != udp::send_text)
            {
                if (m_debug) { std::cerr << "UDP::ReceiveText wrong packet type\n"; }
                return -1;
            }
            packetList[packet.packetCount] = packet;
            len += packet.lenThisUDPPacket;
            if (packetList.size() >= packet.numUDPPackets) break;
        }
    }

    if (len != packet.totalLength || packet.totalLength == 0)
    {
        if (m_debug) { std::cerr << "UDP::ReceiveText len = " << len << " packet.totalLength = " << packet.totalLength << "\n"; }
        return -1;
    }
    text->clear();
    text->reserve(packet.totalLength);
    for (auto &&iter : packetList) { text->append(iter.second.text, iter.second.lenThisUDPPacket); }
    return int(packet.totalLength);
}

// redundancy specifies the number of extra packets to send
int UDP::SendFEC(uint64_t matchID, const struct sockaddr_in &destination, const std::string &data, uint32_t percentRedundancy)
{
#ifdef NON_THREAD_SAFE_OK
    // note that gf can be either unsigned char or unsigned short
    // depending on the size of GF_BITS (2 to 16)
    // GF_SIZE is ((1 << GF_BITS) - 1) so for
    // GF_BITS = 8, GF_SIZE = 255, gf unsigned char
    // GF_BITS = 16, GF_SIZE = 65535, gf unsigned short
    assert(sizeof(gf) == 1 || sizeof(gf) == 2); // this check is just in case we have a compiler with weird unsigned short size
    assert(udp::kUDPPacketTextSize % sizeof(gf) == 0); // this check means that udp::kUDPPacketTextSize is even when sizeof(gf) == 2

    size_t len = data.size();
    size_t k = len / udp::kUDPPacketTextSize;
    if (len % udp::kUDPPacketTextSize) k++;
    size_t n = size_t((k * (100 + percentRedundancy)) / 100);
    if (n <= k) n++;
    if (n >= GF_SIZE) // too big so currently return an error
    {
        if (m_debug) { std::cerr << "UDP::SendFEC n (" << n << ") >= GF_SIZE (" << GF_SIZE << ") \n"; }
        return -1;
    }

    udp::SendFECUDPPacket packet;
    packet.packetID = matchID;
    packet.totalLength = len;
    packet.numUDPPackets = k;
    packet.lenThisUDPPacket = udp::kUDPPacketTextSize;

    struct fec_parms *code = fec_new(int(k), int(n));
    size_t paddedLen = k * udp::kUDPPacketTextSize;
    std::unique_ptr<gf[]> buf = std::make_unique<gf[]>(paddedLen / sizeof(gf));
    std::memcpy(reinterpret_cast<char *>(buf.get()), data.data(), len);
    if (paddedLen != len) std::memset(reinterpret_cast<char *>(buf.get()) + len, 0, paddedLen - len);
    std::unique_ptr<gf*[]> src = std::make_unique<gf*[]>(k);
    for (size_t i = 0; i < k; i++) src[i] = buf.get() + i * udp::kUDPPacketTextSize / sizeof(gf);
    gf *fec;

    for (size_t i = 0; i < n; i++)
    {
        packet.packetCount = i;
        fec = reinterpret_cast<gf *>(packet.text);
        fec_encode(code, src.get(), fec, int(i), udp::kUDPPacketTextSize);
        if (m_debug) { std::cerr << "UDP::SendFEC Sending FEC index " << i  << "\n"; }
        if (SendUDPPacket(destination, &packet, sizeof(udp::SendFECUDPPacket)) == -1)
        {
            if (m_debug) { std::cerr << "SendFEC Error sSendUDPPacket\n"; }
            fec_free(code);
            return -1;
        }
    }
    fec_free(code);
    return int(len);
#else
    return 0;
#endif
}

int UDP::ReceiveFEC(uint64_t matchID, uint32_t percentRedundancy, std::string *data, struct sockaddr_in *sender)
{
#ifdef NON_THREAD_SAFE_OK
    // note that gf can be either unsigned char or unsigned short
    // depending on the size of GF_BITS (2 to 16)
    // GF_SIZE is ((1 << GF_BITS) - 1) so for
    // GF_BITS = 8, GF_SIZE = 255, gf unsigned char
    // GF_BITS = 16, GF_SIZE = 65535, gf unsigned short
    assert(sizeof(gf) == 1 || sizeof(gf) == 2); // this check is just in case we have a compiler with weird unsigned short size
    assert(udp::kUDPPacketTextSize % sizeof(gf) == 0); // this check means that udp::kUDPPacketTextSize is even when sizeof(gf) == 2

    std::map<size_t, udp::SendFECUDPPacket>packetList;
    udp::SendFECUDPPacket packet;

    while (CheckReceiver(1000000) == 1)
    {
        if (ReceiveUDPPacket(&packet, sizeof(udp::SendFECUDPPacket), sender) == -1)
        {
            if (m_debug) { std::cerr << "UDP::ReceiveFEC error in ReceiveUDPPacket\n"; }
            return -1;
        }
        // if matchID is set ignore packets with the wrong ID
        if ((matchID == 0 || packet.packetID == matchID) && packet.type == udp::send_fec)
        {
            packetList[packet.packetCount] = packet;
            if (packetList.size() == packet.numUDPPackets) break;
        }
    }

    if (packetList.size() < packet.numUDPPackets || packet.numUDPPackets == 0)
    {
        if (m_debug) { std::cerr << "UDP::ReceiveFEC " << packetList.size() << " packets received " << packet.numUDPPackets + 1 << " required\n"; }
        return -1;
    }

    size_t k = packet.numUDPPackets;
    size_t n = size_t((k * (100 + percentRedundancy)) / 100);
    if (n <= k) n++;
    if (n >= GF_SIZE) // too big so currently return an error
    {
        if (m_debug) { std::cerr << "UDP::ReceiveFEC n >= GF_SIZE\n"; }
        return -1;
    }

    struct fec_parms *code  = fec_new(int(k), int(n));
    // pkt = new gf *[k];
    std::unique_ptr<gf*[]> pkt = std::make_unique<gf*[]>(k);
    // index = new int[k];
    std::unique_ptr<int[]> index = std::make_unique<int[]>(k);
    size_t j = 0;
    for (auto &&iter : packetList)
    {
        pkt[j] = reinterpret_cast<gf *>(iter.second.text);
        index[j] = int(iter.first);
        j++;
    }

    if (fec_decode(code, pkt.get(), index.get(), udp::kUDPPacketTextSize))
    {
        if (m_debug) { std::cerr << "UDP::ReceiveFEC error in fec_decode " << packetList.size() << " packets received " << packet.numUDPPackets << " required\n"; }
        fec_free(code);
        return -1;
    }

    data->clear();
    data->reserve(udp::kUDPPacketTextSize * k);
    for (size_t i = 0; i < k; i++)
    {
        data->append(reinterpret_cast<const char *>(pkt[i]), udp::kUDPPacketTextSize);
    }
    data->resize(packet.totalLength);
    fec_free(code);

    if (m_debug) { std::cerr << "ReceiveFEC read " << packet.totalLength << " bytes successfully\n"; }
    return int(packet.totalLength);
#else
    return 0;
#endif
}


// check whether there is anything waiting to be received
// set timeout to 0 if want immediate response
int UDP::CheckReceiver(uint64_t usecTimeout)
{
    fd_set fds;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(m_RecSockfd, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = long(usecTimeout / 1000000);
    tv.tv_usec = long(usecTimeout % 1000000);

    // wait until timeout or data received
    // note 1st parameter is ignored on Windows
    int n = select(int(m_RecSockfd + 1), &fds, nullptr, nullptr, &tv);
    if (m_debug) { std::cerr << "UDP::CheckReceiver n = " << n << " m_RecSockfd = " << m_RecSockfd << "\n"; }
    return n; // 0 if nothing ready, 1 if something there, -1 on error
}

void UDP::setDebug(bool debug)
{
    m_debug = debug;
}

UDPUpDown::UDPUpDown()
{
#if defined(_WIN32) || defined(WIN32)
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        std::cerr << "Error in TCP::TCP: Unable to find a usable Winsock DLL " << err;
        m_status = 1;
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2|| HIBYTE(wsaData.wVersion) != 2)
    {
       WSACleanup();
       std::cerr << "Error in TCP::TCP: Version mismatch in Winsock DLL " << err;
       m_status = 2;
       return;
    }
#endif
    m_status = 0;
    return;
}

UDPUpDown::~UDPUpDown()
{
#if defined(_WIN32) || defined(WIN32)
    WSACleanup();
#endif
}

int UDPUpDown::status() const
{
    return m_status;
}

