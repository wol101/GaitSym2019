/*
 *  DataTarget.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 */

#include <iostream>
#include <cfloat>

#include "ode/ode.h"

#include "DataTarget.h"
#include "GSUtil.h"
#include "Simulation.h"

DataTarget::DataTarget()
{
    m_TargetTimeListLength = -1;
    m_TargetTimeList = nullptr;
    m_ValueList = nullptr;
    m_ValueListLength = -1;
    m_Intercept = 0;
    m_Slope = -1;
    m_MatchType = linear;
    m_AbortThreshold = -DBL_MAX;
    m_Target = nullptr;
    m_LastMatchIndex = -1;
}

DataTarget::~DataTarget()
{
    if (TargetTimeList()) delete [] TargetTimeList();
}

void DataTarget::SetTargetTimes(int size, double *targetTimes)
{
    int i;
    if (size <= 0)
    {
        std::cerr << "DataTarget::SetTargetTimes error: size = " << size << "\n";
        return;
    }
    if (TargetTimeListLength() != size)
    {
        if (TargetTimeList()) delete [] TargetTimeList();
        m_TargetTimeListLength = size;
        m_TargetTimeList = new double[TargetTimeListLength()];
    }
    for (i = 0 ; i < TargetTimeListLength(); i++)
    {
        TargetTimeList()[i] = targetTimes[i];
    }
}

void DataTarget::SetTargetValues(int size, double *values)
{
    int i;
    if (size <= 0)
    {
        std::cerr << "DataTarget::SetTargetValues error: size = " << size << "\n";
        return;
    }
    if (ValueListLength() != size)
    {
        if (m_ValueList) delete [] m_ValueList;
        m_ValueListLength = size;
        m_ValueList = new double[ValueListLength()];
    }
    for (i = 0; i < ValueListLength(); i++)
    {
        m_ValueList[i] = values[i];
    }
}


// returns the index if a target match time is triggered
// the tolerance needs to be lower than the distance between target times
// returns -1 if no target is found within the tolerance
int DataTarget::TargetMatch(double time, double tolerance)
{
    m_LastMatchIndex = ProtectedTargetMatch(time, tolerance);
    return m_LastMatchIndex;
}

int DataTarget::ProtectedTargetMatch(double time, double tolerance)
{
    int index = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), time);
    if (index < 0)
    {
        if (fabs(TargetTimeList()[0] - time) <= tolerance) return 0;
        return -1;
    }
    if (index >= TargetTimeListLength() - 1)
    {
        if (fabs(TargetTimeList()[TargetTimeListLength() - 1] - time) <= tolerance) return TargetTimeListLength() - 1;
        return -1;
    }
    if (fabs(TargetTimeList()[index] - time) <= tolerance) return index;
    if (fabs(TargetTimeList()[index + 1] - time) <= tolerance) return index + 1;
    return -1;
}

int DataTarget::ValueListLength() const
{
    return m_ValueListLength;
}

double *DataTarget::ValueList() const
{
    return m_ValueList;
}

double *DataTarget::TargetTimeList() const
{
    return m_TargetTimeList;
}

int DataTarget::TargetTimeListLength() const
{
    return m_TargetTimeListLength;
}

double DataTarget::PositiveFunction(double v)
{
    switch (m_MatchType)
    {
    case linear:
        if (v > 0) return v;
        else return -v;

    case square:
        return v * v;
    }
    return 0;
}

double DataTarget::GetMatchValue(double time)
{
    double matchScore = m_Intercept + m_Slope * PositiveFunction(GetError(time));
    if (matchScore < m_AbortThreshold)
    {
        //std::cerr << "matchScore " << matchScore << "\n";
        simulation()->SetDataTargetAbort(true);
    }
    return matchScore;
}

double DataTarget::GetMatchValue(int index)
{
    double matchScore = m_Intercept + m_Slope * PositiveFunction(GetError(index));
    if (matchScore < m_AbortThreshold)
    {
        //std::cerr << "matchScore " << matchScore << "\n";
        simulation()->SetDataTargetAbort(true);
    }
    return matchScore;
}



