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
#include <algorithm>

using namespace std::string_literals;

DataTarget::DataTarget()
{
}

std::vector<double> *DataTarget::targetTimeList()
{
    return &m_targetTimeList;
}

double DataTarget::lastValue() const
{
    return m_lastValue;
}

double DataTarget::positiveFunction(double v)
{
    switch (m_matchType)
    {
    case Linear:
        return std::abs(v);

    case Square:
        return v * v;

    case Raw:
        return v;
    }
    return 0;
}

std::tuple<double, bool> DataTarget::calculateMatchValue(double time)
{
    switch (m_interpolationType)
    {
    case Punctuated:
        {
            auto lowerBound = std::lower_bound(m_targetTimeList.begin(), m_targetTimeList.end(), time);
            size_t index = std::distance(m_targetTimeList.begin(), lowerBound);
            if (index == 0)
                return std::make_tuple(m_lastValue, false); // this means that time is less than the lowest value in the list
            if (index == m_lastIndex)
                return std::make_tuple(m_lastValue, false);
            m_lastIndex = index;
            m_lastValue = m_intercept + m_slope * positiveFunction(calculateError(index - 1));
            break;
        }
    case Continuous:
        {
            m_lastValue = m_intercept + m_slope * positiveFunction(calculateError(time));
            break;
        }
    }
    if (m_lastValue < m_abortBelow || m_lastValue > m_abortAbove)
    {
        simulation()->SetDataTargetAbort(name());
        m_lastValue += m_abortBonus;
    }
    return std::make_tuple(m_lastValue, true);
}

std::string DataTarget::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tMatchValue\tValid\n";
    }
    double value;
    bool valid;
    std::tie(value, valid) = calculateMatchValue(simulation()->GetTime());
    ss << simulation()->GetTime() << "\t" << value << "\t" << valid << "\n";
//    ss << simulation()->GetTime() << "\t" << std::get<0>(GetMatchValue(simulation()->GetTime())) << "\n";
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
    m_intercept = GSUtil::Double(buf);
    if (findAttribute("Slope"s, &buf) == nullptr) return lastErrorPtr();
    m_slope = GSUtil::Double(buf);

    if (findAttribute("AbortAbove"s, &buf)) m_abortAbove = GSUtil::Double(buf);
    if (findAttribute("AbortBelow"s, &buf)) m_abortBelow = GSUtil::Double(buf);
    if (findAttribute("AbortBonus"s, &buf)) m_abortBonus = GSUtil::Double(buf);

    if (findAttribute("TargetTimes"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetTimesTokens;
    pystring::split(buf, targetTimesTokens);
    if (targetTimesTokens.size() == 0)
    {
        setLastError("DataTarget ID=\""s + name() +"\" No times found in TargetTimes"s);
        return lastErrorPtr();
    }
    m_targetTimeList.clear();
    m_targetTimeList.reserve(targetTimesTokens.size());
    for (auto &&token : targetTimesTokens) m_targetTimeList.push_back(GSUtil::Double(token));
    if (std::is_sorted(m_targetTimeList.begin(), m_targetTimeList.end()) == false)
    {
        setLastError("DataTarget ID=\""s + name() +"\" TargetTimes are not in ascending order"s);
        return lastErrorPtr();
    }

    if (findAttribute("MatchType"s, &buf) == nullptr) return lastErrorPtr();
    size_t matchTypeIndex;
    for (matchTypeIndex = 0; matchTypeIndex < matchTypeCount; matchTypeIndex++)
    {
        if (buf == matchTypeStrings(matchTypeIndex))
        {
            m_matchType = static_cast<MatchType>(matchTypeIndex);
            break;
        }
    }
    if (matchTypeIndex >= matchTypeCount)
    {
        setLastError("DataTarget ID=\""s + name() +"\" Unrecognised MatchType "s + buf);
        return lastErrorPtr();
    }

    if (findAttribute("InterpolationType"s, &buf) == nullptr) return lastErrorPtr();
    size_t interpolationTypeIndex;
    for (interpolationTypeIndex = 0; interpolationTypeIndex < interpolationTypeCount; interpolationTypeIndex++)
    {
        if (buf == interpolationTypeStrings(interpolationTypeIndex))
        {
            m_interpolationType = static_cast<InterpolationType>(interpolationTypeIndex);
            break;
        }
    }
    if (interpolationTypeIndex >= interpolationTypeCount)
    {
        setLastError("DataTarget ID=\""s + name() +"\" Unrecognised InterpolationType "s + buf);
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
    buf.reserve(size_t(m_targetTimeList.size()) * 32);
    setAttribute("Intercept"s, *GSUtil::ToString(m_intercept, &buf));
    setAttribute("Slope"s, *GSUtil::ToString(m_slope, &buf));
    setAttribute("AbortAbove"s, *GSUtil::ToString(m_abortAbove, &buf));
    setAttribute("AbortBelow"s, *GSUtil::ToString(m_abortBelow, &buf));
    setAttribute("AbortBonus"s, *GSUtil::ToString(m_abortBonus, &buf));
    setAttribute("TargetTimes"s, *GSUtil::ToString(m_targetTimeList.data(), m_targetTimeList.size(), &buf));
    setAttribute("MatchType", matchTypeStrings(m_matchType));
    setAttribute("InterpolationType", interpolationTypeStrings(m_interpolationType));
}


