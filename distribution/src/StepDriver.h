/*
 *  StepDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 * Uses a pure stepped function to return a the value at a given time
 *
 */

#ifndef StepDriver_h
#define StepDriver_h

#include "Driver.h"

class StepDriver: public Driver
{
public:
    StepDriver();
    virtual ~StepDriver();

    void SetValuesAndDurations(size_t size, double *values, double *durations);
    virtual double GetValue(double time);

    virtual std::string *CreateFromAttributes();
    virtual void AppendToAttributes();

private:
    double *m_ValueList = nullptr;
    double *m_DurationList = nullptr;
    size_t m_ListLength = 0;
    size_t m_LastIndex = 0;
};

#endif

