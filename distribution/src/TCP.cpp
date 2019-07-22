/*
 *  TCP.cpp
 *  GeneralGA
 *
 *  Created by Bill Sellers on 05/04/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#ifdef USE_TCP

#include "TCP.h"

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <map>
#include <iostream>
#include <iomanip>

#define MAXBUFLEN 1500 // MTU should be <= this
#define BACKLOG 128 // seems a reasonable value

TCP::TCP()
{
}

TCP::~TCP()
{
}

int TCP::StartServer(int port) 
{ 
    if ((m_Sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) return -1;
    
    m_myAddress.sin_family = AF_INET; // host byte order 
    m_myAddress.sin_port = htons(port); // short, network byte order 
    m_myAddress.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP  
    memset(&(m_myAddress.sin_zero), 0, 8); // zero the rest of the struct 
    
    int yes=1;
    // stop irritating reuse messages
    setsockopt(m_Sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    
    if (port != 0)
    {
        if (bind(m_Sockfd, (struct sockaddr *)&m_myAddress, sizeof(struct sockaddr)) == -1) 
        {
            close(m_Sockfd);
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
                close(m_Sockfd);
                return -1;
            }
        }
    }
    
    if (listen(m_Sockfd, BACKLOG) == -1) 
    { 
        close(m_Sockfd);
        return -1; 
    } 
    
    
    int optval = 1; 
    // turn off SIGPIPE signal
#if defined(__linux__) || defined(__CYGWIN__)
    setsockopt(m_Sockfd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
#else
    setsockopt(m_Sockfd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif
    // set SO_LINGER so socket closes gracefully
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 10;
    setsockopt(m_Sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    // disable the Nagle algorithm so that small packets get sent immediately
    int flag = 1;
    setsockopt(m_Sockfd, IPPROTO_TCP, TCP_NODELAY,  &flag, sizeof(int));    
    
#ifdef TCP_DEBUG
    std::cerr << "TCP::StartListener\n";
    std::cerr << "Listening on port " << port << "\n";
#endif
    
    return 0;
}

void TCP::StopServer()
{
    close(m_Sockfd); 
    return;
} 

int TCP::StartAcceptor(int listeningSocket)
{
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr); 
    m_Sockfd = accept(listeningSocket, (struct sockaddr *)&m_senderAddress, &addr_len);
#ifdef TCP_DEBUG
    std::cerr << "TCP::Accept\n";
    std::cerr << "sin_addr.s_addr " << ntohl(m_senderAddress.sin_addr.s_addr) << "\n";
    std::cerr << "sin_port " << ntohs(m_senderAddress.sin_port) << "\n";
#endif
    if (m_Sockfd == -1) return -1;
    
    int optval = 1; 
    // turn off SIGPIPE signal
#if defined(__linux__) || defined(__CYGWIN__)
    setsockopt(m_Sockfd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
#else
    setsockopt(m_Sockfd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif
    // set SO_LINGER so socket closes gracefully
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 10;
    setsockopt(m_Sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    // disable the Nagle algorithm so that small packets get sent immediately
    int flag = 1;
    setsockopt(m_Sockfd, IPPROTO_TCP, TCP_NODELAY,  &flag, sizeof(int));    
    
    return m_Sockfd;
}

void TCP::StopAcceptor()
{
    close(m_Sockfd); 
    return;
} 

int TCP::StartClient(int port, const char *serverAddress) 
{ 
    struct hostent *he; 
    
    if ((he = gethostbyname(serverAddress)) == 0)
    {  
        return -1; 
    } 
    if ((m_Sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }
    
    m_senderAddress.sin_family = AF_INET; // host byte order 
    m_senderAddress.sin_port = htons(port); // short, network byte order 
    m_senderAddress.sin_addr = *((struct in_addr *)he->h_addr); 
    memset(&(m_senderAddress.sin_zero), 0, 8); // zero the rest of the struct 
    if (connect(m_Sockfd, (struct sockaddr *)&m_senderAddress, sizeof(struct sockaddr)) == -1) 
    { 
        close(m_Sockfd);
        return -1; 
    } 
    
    int optval = 1; 
    // turn off SIGPIPE signal
#if defined(__linux__) || defined(__CYGWIN__)
    setsockopt(m_Sockfd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
#else
    setsockopt(m_Sockfd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif
    // set SO_LINGER so socket closes gracefully
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 10;
    setsockopt(m_Sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    // disable the Nagle algorithm so that small packets get sent immediately
    int flag = 1;
    setsockopt(m_Sockfd, IPPROTO_TCP, TCP_NODELAY,  &flag, sizeof(int));    
    
#ifdef TCP_DEBUG
    std::cerr << "TCP::StartClient\n";
#endif
    
    return 0;
}

void TCP::StopClient()
{
    close(m_Sockfd); 
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
    n = select(m_Sockfd + 1, &fds, NULL, NULL, &tv); 
    
    return n; // 0 if nothing ready, 1 if something there, -1 on error
} 

#endif


