/*
 *  TCP.cpp
 *  GeneralGA
 *
 *  Created by Bill Sellers on 05/04/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#include "TCP.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <map>
#include <iostream>
#include <iomanip>

#if defined(_WIN32) || defined(WIN32)
#include <WinSock2.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define MAXBUFLEN 1500 // MTU should be <= this
#define BACKLOG 128 // seems a reasonable value

TCP::TCP()
{
}

TCP::~TCP()
{
}

int TCP::OneOffInitialisation()
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
        return __LINE__;
    }
    if (LOBYTE(wsaData.wVersion) != 2|| HIBYTE(wsaData.wVersion) != 2)
    {
       WSACleanup();
       std::cerr << "Error in TCP::TCP: Version mismatch in Winsock DLL " << err;
       return __LINE__;
    }
#endif
    return 0;
}

void TCP::OneOffCleanup()
{
#if defined(_WIN32) || defined(WIN32)
    WSACleanup();
#endif
}

int TCP::StartServer(int port)
{
#if defined(_WIN32) || defined(WIN32)
    if ((m_Sockfd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) return -1;
#else
    if ((m_Sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) return -1;
#endif

    m_myAddress.sin_family = AF_INET; // host byte order
    m_myAddress.sin_port = htons(port); // short, network byte order
    m_myAddress.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(m_myAddress.sin_zero), 0, 8); // zero the rest of the struct

    int yes=1;
    // stop irritating reuse messages
    setsockopt(m_Sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(int));

    if (port != 0)
    {
        if (bind(m_Sockfd, (struct sockaddr *)&m_myAddress, sizeof(struct sockaddr)) == -1)
        {
#if defined(_WIN32) || defined(WIN32)
            closesocket(m_Sockfd);
#else
            close(m_Sockfd);
#endif
            return -1;
        }
    }
    else
    {
        port = 32768;
        while (true)
        {
            m_myAddress.sin_port = htons(port); // short, network byte order
            if (bind(m_Sockfd, (struct sockaddr *)&m_myAddress, sizeof(struct sockaddr)) == 0) break;
            port++;
            if (port > 65535)
            {
#if defined(_WIN32) || defined(WIN32)
                closesocket(m_Sockfd);
#else
                close(m_Sockfd);
#endif
                return -1;
            }
        }
    }

    if (listen(m_Sockfd, BACKLOG) == -1)
    {
#if defined(_WIN32) || defined(WIN32)
        closesocket(m_Sockfd);
#else
        close(m_Sockfd);
#endif
        return -1;
    }


    int optval = 1;
    // turn off SIGPIPE signal
#if defined(__linux__) || defined(__CYGWIN__)
    setsockopt(m_Sockfd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
#else
#if defined(_WIN32) || defined(WIN32)
    setsockopt(m_Sockfd, SOL_SOCKET, 0, (const char *)&optval, sizeof(optval));
#else
    setsockopt(m_Sockfd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif
#endif
    // set SO_LINGER so socket closes gracefully
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 10;
    setsockopt(m_Sockfd, SOL_SOCKET, SO_LINGER, (const char *)&ling, sizeof(ling));
    // disable the Nagle algorithm so that small packets get sent immediately
    int flag = 1;
    setsockopt(m_Sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(int));

#ifdef TCP_DEBUG
    std::cerr << "TCP::StartListener\n";
    std::cerr << "Listening on port " << port << "\n";
#endif

    return 0;
}

void TCP::StopServer()
{
#if defined(_WIN32) || defined(WIN32)
    closesocket(m_Sockfd);
#else
    close(m_Sockfd);
#endif
    return;
}

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
SOCKET TCP::StartAcceptor(SOCKET listeningSocket)
#else
int TCP::StartAcceptor(int listeningSocket)
#endif
{
#if defined(_WIN32) || defined(WIN32)
    int addr_len;
#else
    socklen_t addr_len;
#endif
    addr_len = sizeof(struct sockaddr);
    m_Sockfd = accept(listeningSocket, (struct sockaddr *)&m_senderAddress, &addr_len);
#ifdef TCP_DEBUG
    std::cerr << "TCP::Accept\n";
    std::cerr << "sin_addr.s_addr " << ntohl(m_senderAddress.sin_addr.s_addr) << "\n";
    std::cerr << "sin_port " << ntohs(m_senderAddress.sin_port) << "\n";
#endif
#if defined(_WIN32) || defined(WIN32)
    if (m_Sockfd == INVALID_SOCKET) return INVALID_SOCKET;
#else
    if (m_Sockfd == -1) return -1;
#endif

    int optval = 1;
    // turn off SIGPIPE signal
#if defined(__linux__) || defined(__CYGWIN__)
    setsockopt(m_Sockfd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
#else
#if defined(_WIN32) || defined(WIN32)
    setsockopt(m_Sockfd, SOL_SOCKET, 0, (const char *)&optval, sizeof(optval));
#else
    setsockopt(m_Sockfd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif
#endif
    // set SO_LINGER so socket closes gracefully
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 10;
    setsockopt(m_Sockfd, SOL_SOCKET, SO_LINGER, (const char *)&ling, sizeof(ling));
    // disable the Nagle algorithm so that small packets get sent immediately
    int flag = 1;
    setsockopt(m_Sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(int));

    return m_Sockfd;
}

void TCP::StopAcceptor()
{
#if defined(_WIN32) || defined(WIN32)
    closesocket(m_Sockfd);
#else
    close(m_Sockfd);
#endif
    return;
}

int TCP::StartClient(int port, const char *serverAddress)
{
    struct hostent *he;

    if ((he = gethostbyname(serverAddress)) == nullptr)
    {
        return -1;
    }
#if defined(_WIN32) || defined(WIN32)
    if ((m_Sockfd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
#else
    if ((m_Sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
#endif
    {
        return -1;
    }

    m_senderAddress.sin_family = AF_INET; // host byte order
    m_senderAddress.sin_port = htons(port); // short, network byte order
    m_senderAddress.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(m_senderAddress.sin_zero), 0, 8); // zero the rest of the struct
    if (connect(m_Sockfd, (struct sockaddr *)&m_senderAddress, sizeof(struct sockaddr)) == -1)
    {
#if defined(_WIN32) || defined(WIN32)
        closesocket(m_Sockfd);
#else
        close(m_Sockfd);
#endif
        return -1;
    }

    int optval = 1;
    // turn off SIGPIPE signal
#if defined(__linux__) || defined(__CYGWIN__)
    setsockopt(m_Sockfd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
#else
#if defined(_WIN32) || defined(WIN32)
    setsockopt(m_Sockfd, SOL_SOCKET, 0, (const char *)&optval, sizeof(optval));
#else
    setsockopt(m_Sockfd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif
#endif
    // set SO_LINGER so socket closes gracefully
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 10;
    setsockopt(m_Sockfd, SOL_SOCKET, SO_LINGER, (const char *)&ling, sizeof(ling));
    // disable the Nagle algorithm so that small packets get sent immediately
    int flag = 1;
    setsockopt(m_Sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(int));

#ifdef TCP_DEBUG
    std::cerr << "TCP::StartClient\n";
#endif

    return 0;
}

void TCP::StopClient()
{
#if defined(_WIN32) || defined(WIN32)
    closesocket(m_Sockfd);
#else
    close(m_Sockfd);
#endif
    return;
}


// return the number of bytes actually sent
int TCP::SendData(char *data, int numBytes)
{
    int total = 0; // how many bytes we have sent
    int bytesleft = numBytes; // how many we have left to send
    int n;
    while(total < numBytes)
    {
        n = send(m_Sockfd, data + total, bytesleft, 0);
        if (n == -1) break;
        total += n;
        bytesleft -= n;
    }

#ifdef TCP_DEBUG
    std::cerr << "TCP::SendData\n";
    std::cerr << "Sent bytes " << total << "\n";
#endif

    return total;
}

// returns number of bytes read
// 0 is an error since it blocks till it gets something
// set timeout to 0 if want immediate response
int TCP::ReceiveData(char *data, int numBytes, long secTimeout, long usecTimeout)
{
    int total = 0; // how many bytes we have sent
    int bytesleft = numBytes; // how many we have left to send
    int n;
    while(total < numBytes)
    {
        if (CheckReceiver(secTimeout, usecTimeout) != 1) break;
        n = recv(m_Sockfd, data + total, bytesleft, 0);
        if (n <= 0) break;
        total += n;
        bytesleft -= n;
    }

#ifdef TCP_DEBUG
    std::cerr << "TCP::ReceiveData\n";
    std::cerr << "Received bytes " << total << "\n";
#endif

    return total;
}


// check whether there is anything waiting to be received
// set timeout to 0 if want immediate response
int TCP::CheckReceiver(long secTimeout, long usecTimeout)
{
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(m_Sockfd, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = secTimeout;
    tv.tv_usec = usecTimeout;

    // wait until timeout or data received
    // note 1st parameter is ignored on Windows
    n = select(int(m_Sockfd + 1), &fds, nullptr, nullptr, &tv);

    return n; // 0 if nothing ready, 1 if something there, -1 on error
}

void TCP::GetSenderAddress(uint32_t *address, uint32_t *port)
{
    // only valid for IP4
    if (m_senderAddress.sin_family == AF_INET)
    {
        *address = ntohl(m_senderAddress.sin_addr.s_addr);
        *port = ntohs(m_senderAddress.sin_port);
    }
    else
    {
        *address = 0;
        *port = 0;
    }
}

void TCP::GetMyAddress(uint32_t *address, uint32_t *port)
{
    // only valid for IP4
    if (m_myAddress.sin_family == AF_INET)
    {
        *address = ntohl(m_myAddress.sin_addr.s_addr);
        *port = ntohs(m_myAddress.sin_port);
    }
    else
    {
        *address = 0;
        *port = 0;
    }
}


