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

extern "C" void ODEMessageTrap(int num, const char *msg, va_list ap);

bool IsMessage();
const char *GetLastMessage(int *messageNumber);

#endif
