/*
 *  TCP.h
 *  GeneralGA
 *
 *  Created by Bill Sellers on 05/04/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Genome;
class TCP;

class TCP
    {
    public:
        TCP();
        virtual ~TCP();

        int StartServer(int MYPORT);
        void StopServer();
        int StartClient(int port, const char *serverAddress);
        void StopClient();
        int StartAcceptor(int listeningSocket);
        void StopAcceptor();
        int ReceiveData(char *data, int numBytes, long secTimeout, long usecTimeout);
        int SendData(char *data, int numBytes);
        int CheckAlive(struct sockaddr_in *destination) { return 0; };
        int CheckReceiver(long secTimeout, long usecTimeout);

        struct sockaddr_in  *GetMyAddress() { return &m_myAddress; };
        struct sockaddr_in  *GetSenderAddress() { return &m_senderAddress; };
        int GetSocket() { return m_Sockfd; };

    private:

        int m_Sockfd;
        struct sockaddr_in m_myAddress; // my address information
        struct sockaddr_in m_senderAddress; // my address information
    };

