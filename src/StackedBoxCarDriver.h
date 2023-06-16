/*
 *  StackedBoxCarDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Thu Feb 21 2013.
 *  Copyright (c) 2013 Bill Sellers. All rights reserved.
 *
 *  Uses a cyclic boxcar function to return a the value at a given time
 *
 */

#ifndef STACKEDBOXCARDRIVER_H
#define STACKEDBOXCARDRIVER_H

#include <vector>
#include "Driver.h"

class StackedBoxcarDriver : public Driver
{
public:
    StackedBoxcarDriver();
    virtual ~StackedBoxcarDriver();

    void SetStackSize(size_t StackSize);
    void SetCycleTime(double CycleTime);
    void SetDelays(double *Delays);
    void SetWidths(double *Widths);
    void SetHeights(double *Heights);

    double GetCycleTime() { return m_CycleTime; }
    std::vector<double> *GetDelays() { return &m_Delays; }
    std::vector<double> *GetWidths() { return &m_Widths; }
    std::vector<double> *GetHeights() { return &m_Heights; }

    virtual void Update();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:
    double m_CycleTime;
    std::vector<double> m_Delays;
    std::vector<double> m_Widths;
    std::vector<double> m_Heights;
    size_t m_StackSize = 0;
};

#endif // STACKEDBOXCARDRIVER_H

