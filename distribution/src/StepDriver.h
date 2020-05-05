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

    virtual void Update();

    void SetValuesAndDurations(size_t size, double *values, double *durations);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:
    std::vector<double> m_ValueList;
    std::vector<double> m_DurationList;
    size_t m_ListLength = 0;
    size_t m_LastIndex = 0;
};

#endif

