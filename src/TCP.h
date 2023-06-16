/*
 *  TCP.h
 *  GeneralGA
 *
 *  Created by Bill Sellers on 05/04/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#include <stdint.h>

class Genome;
class TCP;

class TCP
{
public:
    TCP();
    virtual ~TCP();

    static int OneOffInitialisation();
    static void OneOffCleanup();

    int StartServer(int MYPORT);
    void StopServer();
    int StartClient(int port, const char *serverAddress);
    void StopClient();
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    SOCKET StartAcceptor(SOCKET listeningSocket);
#else
    int StartAcceptor(int listeningSocket);
#endif
    void StopAcceptor();
    int ReceiveData(char *data, int numBytes, long secTimeout, long usecTimeout);
    int SendData(char *data, int numBytes);
    int CheckAlive(struct sockaddr_in * /* destination */ ) { return 0; }
    int CheckReceiver(long secTimeout, long usecTimeout);

    struct sockaddr_in  *GetMyAddress() { return &m_myAddress; }
    struct sockaddr_in  *GetSenderAddress() { return &m_senderAddress; }
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    SOCKET GetSocket() { return m_Sockfd; }
#else
    int GetSocket() { return m_Sockfd; }
#endif

    // these versions change from network order to local order
    void GetSenderAddress(uint32_t *address, uint32_t *port);
    void GetMyAddress(uint32_t *address, uint32_t *port);

private:

    struct sockaddr_in m_myAddress = {}; // my address information
    struct sockaddr_in m_senderAddress = {}; // my address information
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    SOCKET m_Sockfd = INVALID_SOCKET;
#else
    int m_Sockfd = -1;
#endif
};

class TCPStopServerGuard
{
public:
    TCPStopServerGuard(TCP *tcp) { m_tcp = tcp; }
    ~TCPStopServerGuard() { m_tcp->StopServer(); }
private:
    TCP *m_tcp;
};

class TCPStopClientGuard
{
public:
    TCPStopClientGuard(TCP *tcp) { m_tcp = tcp; }
    ~TCPStopClientGuard() { m_tcp->StopClient(); }
private:
    TCP *m_tcp;
};

class TCPStopAcceptorGuard
{
public:
    TCPStopAcceptorGuard(TCP *tcp) { m_tcp = tcp; }
    ~TCPStopAcceptorGuard() { m_tcp->StopAcceptor(); }
private:
    TCP *m_tcp;
};

class TCPUpDown
{
public:
   TCPUpDown()
   {
      m_status = TCP::OneOffInitialisation();
   }
   ~TCPUpDown()
   {
      TCP::OneOffCleanup();
   }

   int status() const { return m_status; }

private:
   int m_status = 0;
};

