/*
 *  TCPIPMessage.h
 *  AsynchronousGA
 *
 *  Created by Bill Sellers on 13/01/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef TCPIPMessage_h
#define TCPIPMessage_h

#include <stdint.h>

struct TCPIPMessage
{
    char text[16];
    uint32_t genomeLength;
    uint32_t xmlLength;
    uint32_t runID;
    uint32_t senderIP;
    uint32_t senderPort;
    uint32_t md5[4];
    double score;
};

#endif
