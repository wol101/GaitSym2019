/*
 *  CyclicDriver.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 *  Uses a cyclic stepped function to return a the value at a given time
 *
 */

#include "CyclicDriver.h"
#include "GSUtil.h"
#include "Simulation.h"

#include <algorithm>
#include <cassert>

using namespace std::string_literals;

CyclicDriver::CyclicDriver()
{
}

CyclicDriver::~CyclicDriver()
{
}

void CyclicDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());

    // account for phase
    // m_PhaseDelay is a relative value (0 to 1) but the time offset needs to be positive
    double cycleTime = m_changeTimes[m_changeTimes.size() - 2];
    double timeOffset = cycleTime * m_PhaseDelay;
    double time = simulation()->GetTime() + timeOffset;
    time = std::fmod(time, cycleTime);

    if (m_index > m_changeTimes.size() - 2) // this should probably never happen
    {
        std::cerr << "Error in CyclicDriver::Update(): m_index should not be > m_changeTimes.size() - 2 m_index = " << m_index << "\n";
        return;
    }

    // this is an optimisation that assumes this routine gets called a lot of times with the same index
    // which it usually does because the integration step size is small
    if (time >= m_changeTimes[m_index + 1] || time < m_changeTimes[m_index])
    {
        // m_changeTimes starts at 0 and ends at DBL_MAX
        // when time >= 0 and time < m_changeTimes[1] upper_bound will return 1
        // when time >= m_changeTimes[1] and time < m_changeTimes[2] upper_bound will return 2
        // etc.
        auto bound = std::upper_bound(m_changeTimes.begin(), m_changeTimes.end(), time);
        m_index = std::distance(m_changeTimes.begin(), bound) - 1;
    }

    if (Interp() == false)
    {
        if (m_index < m_valueList.size())
            setValue(m_valueList[m_index]);
        else
            setValue(m_valueList.back());
    }
    else
    {
        if (m_index < m_valueList.size() - 1)
            setValue(((time - m_changeTimes[m_index]) / (m_changeTimes[m_index + 1] - m_changeTimes[m_index])) * (m_valueList[m_index + 1] - m_valueList[m_index]) + m_valueList[m_index]);
        else
            setValue(m_valueList.back());
    }
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *CyclicDriver::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    buf.reserve(100000);
    if (findAttribute("Values"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<double> values;
    GSUtil::Double(buf, &values);
    if (findAttribute("Durations"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<double> durations;
    GSUtil::Double(buf, &durations);
    if (values.size() != durations.size())
    {
        setLastError("CyclicDriver ID=\""s + name() + "\" number of values ("s + std::to_string(values.size()) + ") must match number of durations ("s + std::to_string(durations.size()) + ")"s);
        return lastErrorPtr();
    }
    m_valueList = values;
    m_durationList = durations;
    m_changeTimes.resize(m_durationList.size() + 2);
    m_changeTimes[0] = 0;
    for (size_t i =0; i < m_durationList.size(); i++) m_changeTimes[i + 1] = m_changeTimes[i] + m_durationList[i];
    m_changeTimes[m_durationList.size() + 1] = DBL_MAX;

    if (findAttribute("PhaseDelay"s, &buf) == nullptr) return lastErrorPtr();
    m_PhaseDelay =  GSUtil::Double(buf);

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void CyclicDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    buf.reserve(m_durationList.size() * 32); // should be big enough but it will grow if necessary anyway
    setAttribute("Type"s, "Cyclic"s);
    setAttribute("Durations"s, *GSUtil::ToString(m_durationList.data(), m_durationList.size(), &buf));
    setAttribute("Values"s, *GSUtil::ToString(m_valueList.data(), m_valueList.size(), &buf));
    setAttribute("PhaseDelay"s, *GSUtil::ToString(m_PhaseDelay, &buf));
}

std::vector<double> CyclicDriver::valueList() const
{
    return m_valueList;
}

void CyclicDriver::setValueList(const std::vector<double> &valueList)
{
    m_valueList = valueList;
}

std::vector<double> CyclicDriver::durationList() const
{
    return m_durationList;
}

void CyclicDriver::setDurationList(const std::vector<double> &durationList)
{
    m_durationList = durationList;
}

double CyclicDriver::GetCycleTime() const
{
    double cycleTime = 0;
    for (auto &&duration : m_durationList)
    {
        cycleTime += duration;
    }
    return cycleTime;
}

