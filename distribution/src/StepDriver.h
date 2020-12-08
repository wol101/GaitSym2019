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

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    std::vector<double> valueList() const;
    void setValueList(const std::vector<double> &valueList);

    std::vector<double> durationList() const;
    void setDurationList(const std::vector<double> &durationList);

private:
    std::vector<double> m_valueList;
    std::vector<double> m_durationList;
    std::vector<double> m_changeTimes;
    size_t m_index = 0;
};

#endif

