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

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    std::vector<double> valueList() const;
    void setValueList(const std::vector<double> &valueList);

    std::vector<double> durationList() const;
    void setDurationList(const std::vector<double> &durationList);

    double GetCycleTime() const;

private:

    std::vector<double> m_valueList;
    std::vector<double> m_durationList;
    std::vector<double> m_changeTimes;
    double m_PhaseDelay = 0;
    size_t m_index = 0;
};

#endif
