/*
 *  CyclicDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 *  Uses a cyclic stepped function to return a the value at a given time
 *
 */

#ifndef CyclicDriver_h
#define CyclicDriver_h

#include "Driver.h"

class CyclicDriver: public Driver
{
public:
    CyclicDriver();
    virtual ~CyclicDriver();

    virtual void Update();

    void SetValuesAndDurations(int size, double *values, double *durations);
    void SetPhaseDelay(double phaseDelay); // 0 to 1
    double GetCycleTime();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    std::vector<double> m_ValueList;
    std::vector<double> m_DurationList;
    double m_PhaseDelay = {0};
    int m_ListLength = {-1};
    int m_LastIndex = {0};
};

#endif
