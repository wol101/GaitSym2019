/*
 *  ErrorHandler.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 14/02/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include "ErrorHandler.h"

static char gMessageText[1024] = "";
static int gMessageNumber = 0;
static int gMessageFlag = false;

extern "C" void ODEMessageTrap(int num, const char *msg, va_list ap)
{
    fflush (stderr);
    fflush (stdout);
    fprintf (stderr,"\n%d: ", num);
    vfprintf (stderr, msg, ap);
    fprintf (stderr, "\n");
    fflush (stderr);

    vsprintf (gMessageText, msg, ap);
    gMessageNumber = num;
    gMessageFlag = true;
}

bool IsMessage()
{
    bool error = gMessageFlag;
    gMessageFlag = false;
    return error;
}

const char *GetLastMessage(int *messageNumber)
{
    if (messageNumber) *messageNumber = gMessageNumber;
    return gMessageText;
}

