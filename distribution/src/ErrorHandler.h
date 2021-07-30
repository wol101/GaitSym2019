/*
 *  ErrorHandler.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 14/02/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#ifndef ErrorHandler_h
#define ErrorHandler_h

#include <cstdarg>
#include <string>

class ErrorHandler
{
public:
    static void ODEMessageTrap(int num, const char *msg, va_list ap);
    static bool IsMessage();
    static std::string GetLastMessage();
    static int GetLastMessageNumber();
    static void ClearMessage();

private:
    static std::string m_messageText;
    static int m_messageNumber;
    static bool m_messageFlag;
};

#endif
