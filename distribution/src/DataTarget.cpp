/*
 *  DataTarget.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 */

#include "DataTarget.h"
#include "Simulation.h"
#include "GSUtil.h"

#include "pystring.h"

#include <iostream>
#include <cfloat>
#include <sstream>

using namespace std::string_literals;

DataTarget::DataTarget()
{
}

DataTarget::~DataTarget()
{
}

void DataTarget::SetTargetTimes(int size, double *targetTimes)
{
    if (size <= 0)
    {
        std::cerr << "DataTarget::SetTargetTimes error: size = " << size << "\n";
        return;
    }
    m_TargetTimeListLength = size;
    m_TargetTimeList.resize(size_t(m_TargetTimeListLength));
    for (size_t i = 0 ; i < size_t(m_TargetTimeListLength); i++)
    {
        m_TargetTimeList[i] = targetTimes[i];
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
    int index = GSUtil::BinarySearchRange(m_TargetTimeList.data(), m_TargetTimeListLength, time);
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

const double *DataTarget::TargetTimeList() const
{
    return m_TargetTimeList.data();
}

int DataTarget::TargetTimeListLength() const
{
    return m_TargetTimeListLength;
}

double DataTarget::PositiveFunction(double v)
{
    switch (m_MatchType)
    {
    case Linear:
        if (v > 0) return v;
        else return -v;

    case Square:
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

int DataTarget::GetLastMatchIndex()
{
    return m_LastMatchIndex;
}

void DataTarget::SetIntercept(double intercept)
{
    m_Intercept = intercept;
}

void DataTarget::SetSlope(double slope)
{
    m_Slope = slope;
}

void DataTarget::SetMatchType(MatchType t)
{
    m_MatchType = t;
}

void DataTarget::SetAbortThreshold(double a)
{
    m_AbortThreshold = a;
}

std::string DataTarget::dump()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (getFirstDump())
    {
        setFirstDump(false);
        ss << "Time\tMatchValue\n";
    }
    ss << simulation()->GetTime() << "\t" << GetMatchValue(simulation()->GetTime()) << "\n";
    return ss.str();
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *DataTarget::createFromAttributes()
{
    if (NamedObject::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    buf.reserve(10000);

    if (findAttribute("Intercept"s, &buf) == nullptr) return lastErrorPtr();
    m_Intercept = GSUtil::Double(buf);
    if (findAttribute("Slope"s, &buf) == nullptr) return lastErrorPtr();
    m_Slope = GSUtil::Double(buf);

    if (findAttribute("AbortThreshold"s, &buf)) m_AbortThreshold = GSUtil::Double(buf);

    if (findAttribute("TargetTimes"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetTimesTokens;
    pystring::split(buf, targetTimesTokens);
    if (targetTimesTokens.size() == 0)
    {
        setLastError("DataTarget ID=\""s + name() +"\" No times found in TargetTimes"s);
        return lastErrorPtr();
    }
    std::vector<double> targetTimes;
    targetTimes.reserve(targetTimesTokens.size());
    for (auto token : targetTimesTokens) targetTimes.push_back(GSUtil::Double(token));
    SetTargetTimes(int(targetTimes.size()), targetTimes.data());

    if (findAttribute("MatchType"s, &buf) == nullptr) return lastErrorPtr();
    size_t matchTypeIndex;
    for (matchTypeIndex = 0; matchTypeIndex < matchTypeCount; matchTypeIndex++)
    {
        if (buf == matchTypeStrings(matchTypeIndex))
        {
            m_MatchType = static_cast<MatchType>(matchTypeIndex);
            break;
        }
    }
    if (matchTypeIndex >= matchTypeCount)
    {
        setLastError("DataTarget ID=\""s + name() +"\" Unrecognised MatchType "s + buf);
        return lastErrorPtr();
    }

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void DataTarget::saveToAttributes()
{
    this->setTag("DATATARGET"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void DataTarget::appendToAttributes()
{
    NamedObject::appendToAttributes();
    std::string buf;
    buf.reserve(size_t(m_TargetTimeListLength) * 32);
    setAttribute("Intercept"s, *GSUtil::ToString(m_Intercept, &buf));
    setAttribute("Slope"s, *GSUtil::ToString(m_Slope, &buf));
    setAttribute("AbortThreshold"s, *GSUtil::ToString(m_AbortThreshold, &buf));
    setAttribute("TargetTimes"s, *GSUtil::ToString(m_TargetTimeList.data(), size_t(m_TargetTimeListLength), &buf));
    setAttribute("MatchType", matchTypeStrings(m_MatchType));
}


