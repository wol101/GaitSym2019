/*
 *  UDP.cpp
 *  GeneralGA
 *
 *  Created by Bill Sellers on 30/06/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#ifdef USE_UDP

#include "UDP.h"

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <map>
#include <iostream>
#include <iomanip>

#include "FEC.h"

#define MAXBUFLEN 1500 // MTU should be <= this

UDP::UDP()
{
    m_packet = (UDPPacket *)malloc(sizeof(SendTextUDPPacket));
    init_fec();
}

UDP::~UDP()
{
    free(m_packet);
}

int UDP::StartListener(int port) 
{ 
    if ((m_RecSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) return -1;
    
    m_myAddress.sin_family = AF_INET; // host byte order 
    m_myAddress.sin_port = htons(port); // short, network byte order 
    m_myAddress.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP  
    memset(&(m_myAddress.sin_zero), 0, 8); // zero the rest of the struct 
    
    if (port != 0)
    {
        if (bind(m_RecSockfd, (struct sockaddr *)&m_myAddress, sizeof(struct sockaddr)) == -1) 
        {
            close(m_RecSockfd);
            return -1;
        }
    }
    else
    {
        port = 32768;
        while (true)
        {
            m_myAddress.sin_port = htons(port); // short, network byte order 
            if (bind(m_RecSockfd, (struct sockaddr *)&m_myAddress, sizeof(struct sockaddr)) == 0) break;
            port++;
            if (port > 65535)
            {
                close(m_RecSockfd);
                return -1;
            }
        }
        
    }

    long randomSeed = (long)time(0) * (long)getpid();
    srand(randomSeed);
    m_packetID = (unsigned long)rand();
    if (m_packetID == 0) m_packetID++;

#ifdef UDP_DEBUG
    std::cerr << "UDP::StartListener\n";
    std::cerr << "Listening on port " << port << "\n";
#endif
    
    return 0;
}

void UDP::StopListener()
{
    close(m_RecSockfd); 
    return;
} 

int UDP::StartTalker() 
{ 
    if ((m_SendSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) return -1;
#ifdef UDP_DEBUG
    std::cerr << "UDP::StartTalker\n";
#endif
    return 0;
}

void UDP::StopTalker()
{
    close(m_SendSockfd); 
    return;
} 

int UDP::ReceiveUDPPacket(int numBytes)
{
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr); 
    numBytes = recvfrom(m_RecSockfd, m_packet, numBytes, 0, (struct sockaddr *)&m_senderAddress, &addr_len); 
#ifdef UDP_DEBUG
    std::cerr << "UDP::ReceiveUDPPacket\n";
    std::cerr << "sin_addr.s_addr " << ntohl(m_senderAddress.sin_addr.s_addr) << "\n";
    std::cerr << "sin_port " << ntohs(m_senderAddress.sin_port) << "\n";
    std::cerr << "numBytes " << numBytes << "\n";
    std::cerr << "type " << m_packet->type << "\n";
    std::cerr << "packetID " << m_packet->packetID << "\n";
    switch (m_packet->type)
    {
        case request_send_genome:
            std::cerr << "port " << ((RequestSendGenomeUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((RequestSendGenomeUDPPacket *)m_packet)->index  << "\n";
            std::cerr << "packetNumber " << ((RequestSendGenomeUDPPacket *)m_packet)->packetNumber  << "\n";
            break;
        case genome_received:
            std::cerr << "port " << ((GenomeReceivedUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((GenomeReceivedUDPPacket *)m_packet)->index  << "\n";
            break;
        case send_result:
            std::cerr << "port " << ((SendResultUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((SendResultUDPPacket *)m_packet)->index  << "\n";
            std::cerr << "result " << ((SendResultUDPPacket *)m_packet)->result  << "\n";
            break;
        case send_text:
            std::cerr << "port " << ((SendTextUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((SendTextUDPPacket *)m_packet)->index  << "\n";
            std::cerr << "totalLength " << ((SendTextUDPPacket *)m_packet)->totalLength  << "\n";
            std::cerr << "numUDPPackets " << ((SendTextUDPPacket *)m_packet)->numUDPPackets  << "\n";
            std::cerr << "packetCount " << ((SendTextUDPPacket *)m_packet)->packetCount  << "\n";
            std::cerr << "lenThisUDPPacket " << ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket  << "\n";
            std::cerr << "text " << std::setw(((SendTextUDPPacket *)m_packet)->lenThisUDPPacket) << ((SendTextUDPPacket *)m_packet)->text  << "\n";
            break;
    }
#endif
    return numBytes;
}

int UDP::SendUDPPacket(struct sockaddr_in *destination, int numBytes)
{
    numBytes = sendto(m_SendSockfd, m_packet, numBytes, 0, (struct sockaddr *)destination, sizeof(struct sockaddr));
#ifdef UDP_DEBUG
    std::cerr << "UDP::SendUDPPacket\n";
    std::cerr << "sin_addr.s_addr " << ntohl(destination->sin_addr.s_addr) << "\n";
    std::cerr << "sin_port " << ntohs(destination->sin_port) << "\n";
    std::cerr << "numBytes " << numBytes << "\n";
    std::cerr << "type " << m_packet->type << "\n";
    std::cerr << "packetID " << m_packet->packetID << "\n";
    switch (m_packet->type)
    {
        case request_send_genome:
            std::cerr << "port " << ((RequestSendGenomeUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((RequestSendGenomeUDPPacket *)m_packet)->index  << "\n";
            std::cerr << "packetNumber " << ((RequestSendGenomeUDPPacket *)m_packet)->packetNumber  << "\n";
            break;
        case genome_received:
            std::cerr << "port " << ((GenomeReceivedUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((GenomeReceivedUDPPacket *)m_packet)->index  << "\n";
            break;
        case send_result:
            std::cerr << "port " << ((SendResultUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((SendResultUDPPacket *)m_packet)->index  << "\n";
            std::cerr << "result " << ((SendResultUDPPacket *)m_packet)->result  << "\n";
            break;
        case send_text:
            std::cerr << "port " << ((SendTextUDPPacket *)m_packet)->port  << "\n";
            std::cerr << "index " << ((SendTextUDPPacket *)m_packet)->index  << "\n";
            std::cerr << "totalLength " << ((SendTextUDPPacket *)m_packet)->totalLength  << "\n";
            std::cerr << "numUDPPackets " << ((SendTextUDPPacket *)m_packet)->numUDPPackets  << "\n";
            std::cerr << "packetCount " << ((SendTextUDPPacket *)m_packet)->packetCount  << "\n";
            std::cerr << "lenThisUDPPacket " << ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket  << "\n";
            std::cerr << "text " << std::setw(((SendTextUDPPacket *)m_packet)->lenThisUDPPacket) << ((SendTextUDPPacket *)m_packet)->text  << "\n";
            break;
    }
#endif
    return numBytes;
}

int UDP::SendText(struct sockaddr_in *destination, int index, char *text, int len)
{
    char *p1 = text;
    int bytesToSend = len;
    int numBytes;
    ((SendTextUDPPacket *)m_packet)->type = send_text;
    ((SendTextUDPPacket *)m_packet)->totalLength = len;
    ((SendTextUDPPacket *)m_packet)->numUDPPackets = len / kUDPPacketTextSize;
    if (len % kUDPPacketTextSize) ((SendTextUDPPacket *)m_packet)->numUDPPackets++;
    ((SendTextUDPPacket *)m_packet)->packetCount = 0;
    ((SendTextUDPPacket *)m_packet)->index = index;
    
    while (bytesToSend)
    {
        if (bytesToSend >= kUDPPacketTextSize) ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket = kUDPPacketTextSize;
        else ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket = bytesToSend;
        memcpy(((SendTextUDPPacket *)m_packet)->text, p1, ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket);
#ifdef UDP_DEBUG
        std::cerr << "UDP::SendText\n";
        std::cerr << "Sending " << ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket  << "\n";
#endif
        if ((numBytes = SendUDPPacket(destination, sizeof(SendTextUDPPacket))) == -1) 
        {
#ifdef UDP_DEBUG
            std::cerr << "Error sending text\n";
#endif
            return -1;
        }
        p1 += ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket;
        bytesToSend -= ((SendTextUDPPacket *)m_packet)->lenThisUDPPacket;
        ((SendTextUDPPacket *)m_packet)->packetCount++;
    }
    
    return len;
}

int UDP::ReceiveText(char **buf, unsigned long matchID)
{
    std::map<int, SendTextUDPPacket *>packetList;
    std::map<int, SendTextUDPPacket *>::iterator iter;
    SendTextUDPPacket *p;
    int len = 0;
    
    try
    {
        while (true)
        {
            if (CheckReceiver(1000000) != 1) throw __LINE__;
            ReceiveUDPPacket(sizeof(SendTextUDPPacket));
            p = (SendTextUDPPacket *)GetUDPPacket();
            if (matchID && p->packetID == matchID) // ignore packets with the wrong ID
            {
                if (((SendTextUDPPacket *)m_packet)->type != send_text) throw __LINE__;
                p = new SendTextUDPPacket();
                memcpy(p, ((SendTextUDPPacket *)m_packet), sizeof(SendTextUDPPacket));
                packetList[p->packetCount] = p;
                len += p->lenThisUDPPacket;
                if (packetList.size() >= p->numUDPPackets) break;
            }
        }
        
        if (len != p->totalLength) throw __LINE__;
        *buf = new char[p->totalLength];
        char *ptr = *buf;
        for (iter = packetList.begin(); iter != packetList.end(); iter++)
        {
            memcpy(ptr, iter->second->text, iter->second->lenThisUDPPacket);
            delete iter->second;
            ptr += iter->second->lenThisUDPPacket;
        }
        return len;
    }
    
    catch (int e)
    {
#ifdef UDP_DEBUG
        std::cerr << "UDP::ReceiveText\n";
        std::cerr << "Error receiving text on line " << e << "\n";
#endif
        for (iter = packetList.begin(); iter != packetList.end(); iter++)
        {
            delete iter->second;
        }
        return -1;
    }
    
}

// redundancy specifies the number of extra packets to send
int UDP::SendFEC(struct sockaddr_in *destination, int index, char *data, int len, int percentRedundancy)
{
    int i;
    int numBytes;
    int k = len / kUDPPacketTextSize;
    if (len % kUDPPacketTextSize) k++;
    int n = (k * percentRedundancy) / 100;
    if (n <= k) n++;
    
    if (n >= GF_SIZE) // too big so current return an error
    {
        return -1;
    }
    
    ((SendFECUDPPacket *)m_packet)->type = send_fec;
    ((SendFECUDPPacket *)m_packet)->totalLength = len;
    ((SendFECUDPPacket *)m_packet)->numUDPPackets = k;
    ((SendFECUDPPacket *)m_packet)->index = index;
    ((SendFECUDPPacket *)m_packet)->lenThisUDPPacket = kUDPPacketTextSize;
    
    struct fec_parms *code = fec_new(k, n);
    
    int paddedLen = k * kUDPPacketTextSize;
    gf *buf = new gf[paddedLen];
    memcpy(buf, data, len);
    memset(buf + len, 0, paddedLen - len);
    gf **src = new gf *[k];
    for (i = 0; i < k; i++) src[i] = buf + i * kUDPPacketTextSize;   
    gf *fec;
    
    for (i = 0; i < n; i++)
    {
        ((SendFECUDPPacket *)m_packet)->packetCount = i;
        fec = (gf *)((SendFECUDPPacket *)m_packet)->text;
        fec_encode(code, src, fec, i, kUDPPacketTextSize);

#ifdef UDP_DEBUG
        std::cerr << "UDP::SendFEC\n";
        std::cerr << "Sending FEC index " << i  << "\n";
#endif
        if ((numBytes = SendUDPPacket(destination, sizeof(SendFECUDPPacket))) == -1) 
        {
#ifdef UDP_DEBUG
            std::cerr << "Error sending text\n";
#endif
            delete [] buf;
            delete [] src;
            fec_free(code);
            return -1;
        }
    }
        
    delete [] buf;
    delete [] src;
    fec_free(code);
    return len;
}

int UDP::ReceiveFEC(char **buf, unsigned long matchID, int percentRedundancy)
{
    std::map<int, SendFECUDPPacket *>packetList;
    std::map<int, SendFECUDPPacket *>::iterator iter;
    SendFECUDPPacket *p;
    int i, j;
    int status;
    gf **pkt = 0;
    int *index = 0;
    struct fec_parms *code = 0;
    
    try
    {
        while (true)
        {
            if (CheckReceiver(1000000) != 1) throw __LINE__;
            ReceiveUDPPacket(sizeof(SendFECUDPPacket));
            p = (SendFECUDPPacket *)GetUDPPacket();
            if (matchID && p->packetID == matchID) // ignore packets with the wrong ID
            {
                if (((SendFECUDPPacket *)m_packet)->type != send_fec) throw __LINE__;
                p = new SendFECUDPPacket();
                memcpy(p, ((SendFECUDPPacket *)m_packet), sizeof(SendFECUDPPacket));
                packetList[p->packetCount] = p;
                if (packetList.size() == p->numUDPPackets) break;
            }
        }
        
        int k = p->numUDPPackets;
        int n = (k * percentRedundancy) / 100;
        if (n <= k) n++;
        
        if (n >= GF_SIZE) throw __LINE__;
        
        code = fec_new(k, n);
        pkt = new gf *[k];
        index = new int[k];
        j = 0;
        for (iter = packetList.begin(); iter != packetList.end(); iter++)
        {
            i = iter->first;
            pkt[j] = (gf *)iter->second->text;
            index[j] = i;
            j++;
        }
        
        status = fec_decode(code, pkt, index, kUDPPacketTextSize);
        if (status) throw __LINE__;
        
        *buf = new char[kUDPPacketTextSize * k];
        char *ptr = *buf;
        for (i = 0; i < k; i++)
        {
            memcpy(ptr, pkt[i], kUDPPacketTextSize);
            ptr += kUDPPacketTextSize;
        }

        for (iter = packetList.begin(); iter != packetList.end(); iter++)
        {
            delete iter->second;
        }
        delete [] pkt;
        delete [] index;
        fec_free(code);
        
        return p->totalLength;
    }
    
    catch (int e)
    {
#ifdef UDP_DEBUG
        std::cerr << "UDP::ReceiveFEC\n";
        std::cerr << "Error receiving text on line " << e << "\n";
#endif
        for (iter = packetList.begin(); iter != packetList.end(); iter++)
        {
            delete iter->second;
        }
        if (pkt) delete [] pkt;
        if (index) delete [] index;
        if (code) fec_free(code);
        return -1;
    }
    
}

// check whether there is anything waiting to be received
// set timeout to 0 if want immediate response
int UDP::CheckReceiver(long usecTimeout)
{ 
    fd_set fds; 
    int n; 
    struct timeval tv; 
    
    // set up the file descriptor set 
    FD_ZERO(&fds); 
    FD_SET(m_RecSockfd, &fds); 
    
    // set up the struct timeval for the timeout 
    tv.tv_sec = usecTimeout / 1000000; 
    tv.tv_usec = usecTimeout % 1000000; 
    
    // wait until timeout or data received 
    n = select(m_RecSockfd + 1, &fds, NULL, NULL, &tv); 
#ifdef UDP_DEBUG
        std::cerr << "UDP::CheckReceiver\n";
        std::cerr << "n = " << n << " m_RecSockfd = " << m_RecSockfd << "\n";
#endif
    return n; // 0 if nothing ready, 1 if something there, -1 on error
} 

#endif
