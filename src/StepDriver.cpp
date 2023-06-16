/*
 *  StepDriver.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 * Uses a pure stepped function to return a the value at a given time
 *
 */

#include "StepDriver.h"
#include "GSUtil.h"
#include "Simulation.h"

#include "pystring.h"

#include <algorithm>

using namespace std::string_literals;

StepDriver::StepDriver()
{
}

StepDriver::~StepDriver()
{
}

// optimised assuming the integration step is smaller than the time
// interval (which it always should be)
// this routine assumes that the time is always greater than or
// equal to the previously requested time since this greatly speeds
// up the search since it only ever has to check 2 values
void StepDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());
    double time = simulation()->GetTime();

    // this is an optimisation that assumes this routine gets called a lot of times with the same index
    // which it usually does because the integration step size is small
    if (m_index < m_changeTimes.size() - 1 && (time >= m_changeTimes[m_index + 1] || time < m_changeTimes[m_index]))
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
std::string *StepDriver::createFromAttributes()
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
        setLastError("StepDriver ID=\""s + name() + "\" number of values ("s + std::to_string(values.size()) + ") must match number of durations ("s + std::to_string(durations.size()) + ")"s);
        return lastErrorPtr();
    }
    m_valueList = values;
    m_durationList = durations;
    m_changeTimes.resize(m_durationList.size() + 2);
    m_changeTimes[0] = 0;
    for (size_t i =0; i < m_durationList.size(); i++) m_changeTimes[i + 1] = m_changeTimes[i] + m_durationList[i];
    m_changeTimes[m_durationList.size() + 1] = DBL_MAX;
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void StepDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    buf.reserve(m_durationList.size() * 32); // should be big enough but it will grow if necessary anyway
    setAttribute("Type"s, "Step"s);
    setAttribute("Durations"s, *GSUtil::ToString(m_durationList.data(), m_durationList.size(), &buf));
    setAttribute("Values"s, *GSUtil::ToString(m_valueList.data(), m_valueList.size(), &buf));
}

std::vector<double> StepDriver::valueList() const
{
    return m_valueList;
}

void StepDriver::setValueList(const std::vector<double> &valueList)
{
    m_valueList = valueList;
}

std::vector<double> StepDriver::durationList() const
{
    return m_durationList;
}

void StepDriver::setDurationList(const std::vector<double> &durationList)
{
    m_durationList = durationList;
}


