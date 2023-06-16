/*
 *  ErrorHandler.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 14/02/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include "ErrorHandler.h"

#include <stdio.h>
#include <vector>
#include <functional>

//std::string ErrorHandler::m_messageText;
//int ErrorHandler::m_messageNumber = 0;
//bool ErrorHandler::m_messageFlag = false;

void ErrorHandler::ODEMessageTrap(int num, const char *msg, va_list ap)
{
    fflush (stderr);
    fflush (stdout);
    fprintf (stderr,"\n%d: ", num);
    vfprintf (stderr, msg, ap);
    fprintf (stderr, "\n");
    fflush (stderr);

    // reliably acquire the size
    // from a copy of the variable argument array
    // and a functionally reliable call to mock the formatting
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, ap);
    const int iLen = std::vsnprintf(NULL, 0, msg, vaArgsCopy);
    va_end(vaArgsCopy);

    // return a formatted string without risking memory mismanagement
    // and without assuming any compiler or platform specific behavior
    std::vector<char> zc(iLen + 1);
    std::vsnprintf(zc.data(), zc.size(), msg, ap);
    va_end(ap);

    m_messageText.assign(zc.data(), iLen);
    m_messageNumber = num;
    m_messageFlag = true;
}

bool ErrorHandler::IsMessage()
{
    return m_messageFlag;
}

std::string ErrorHandler::GetLastMessage()
{
    return m_messageText;
}

int ErrorHandler::GetLastMessageNumber()
{
    return m_messageNumber;
}

void ErrorHandler::ClearMessage()
{
    m_messageFlag = false;
    m_messageText.clear();
    m_messageNumber = 0;
}

