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
#include <functional>

// this is glue to allow a C++ function (even a member function) to be called as a C callback
template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }
    static std::function<Ret(Params...)> func;
};

// Initialize the static member.
template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

class ErrorHandler
{
public:
    void ODEMessageTrap(int num, const char *msg, va_list ap);
    bool IsMessage();
    std::string GetLastMessage();
    int GetLastMessageNumber();
    void ClearMessage();

private:
    std::string m_messageText;
    int m_messageNumber = 0;
    bool m_messageFlag = false;
};

#endif
