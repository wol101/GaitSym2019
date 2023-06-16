/*
 *  SimpleStrap.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 21/04/2006.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef SimpleStrap_h
#define SimpleStrap_h

#include "Strap.h"

class Body;

class SimpleStrap:public Strap
{
public:

    virtual void Calculate(double deltaT)
    {
        if (deltaT)
        {
            m_velocity = (m_length - m_LastLength) / deltaT;
        }
        else
        {
            m_velocity = 0;
        }
        m_LastLength = m_length;
    };

    void SetLength(double length) { m_length = length; };


};

#endif
