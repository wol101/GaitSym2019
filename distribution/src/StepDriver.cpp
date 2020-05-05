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

#include <algorithm>

using namespace std::string_literals;

StepDriver::StepDriver()
{
}

StepDriver::~StepDriver()
{
}

// Note list is t0, v0, t1, v1, t2, v2 etc
// times are absolute simulation times not intervals
void StepDriver::SetValuesAndDurations(size_t size, double *values, double *durations)
{
    if (size == 0)
    {
        std::cerr << "StepDriver::SetValueDurationPairs error: size = " << size << "\n";
        return;
    }
    m_ListLength = size;
    m_DurationList.resize(size_t(m_ListLength));
    m_ValueList.resize(size_t(m_ListLength));
    for (size_t i = 0 ; i < m_ListLength; i++)
    {
        m_DurationList[i] = durations[i];
        m_ValueList[i] = values[i];
    }
    m_LastIndex = 0;
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

    if (Interp() == false)
    {
        if (m_LastIndex >= m_ListLength - 1) setValue(m_ValueList[m_ListLength - 1]);
        else
        {
            if (time < m_DurationList[m_LastIndex + 1]) setValue(m_ValueList[m_LastIndex]);
            else
            {

                while (true)
                {
                    m_LastIndex++;
                    if (m_LastIndex >= m_ListLength - 1)
                    {
                        setValue(m_ValueList[m_ListLength - 1]);
                        break;
                    }
                    if (time < m_DurationList[m_LastIndex + 1])
                    {
                        setValue(m_ValueList[m_LastIndex]);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if (m_LastIndex >= m_ListLength - 1) setValue(m_ValueList[m_ListLength - 1]);
        else
        {
            if (time < m_DurationList[m_LastIndex + 1])
            {
                setValue(((time - m_DurationList[m_LastIndex]) / (m_DurationList[m_LastIndex + 1] - m_DurationList[m_LastIndex])) * (m_ValueList[m_LastIndex + 1] - m_ValueList[m_LastIndex]) + m_ValueList[m_LastIndex]);
            }
            else
            {
                while (true)
                {
                    m_LastIndex++;
                    if (m_LastIndex >= m_ListLength - 1)
                    {
                        setValue(m_ValueList[m_ListLength - 1]);
                        break;
                    }
                    if (time < m_DurationList[m_LastIndex + 1])
                    {
                        setValue(((time - m_DurationList[m_LastIndex]) / (m_DurationList[m_LastIndex + 1] - m_DurationList[m_LastIndex])) * (m_ValueList[m_LastIndex + 1] - m_ValueList[m_LastIndex]) + m_ValueList[m_LastIndex]);
                        break;
                    }
                }
            }
        }
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
    if (findAttribute("Steps"s, &buf) == nullptr)
    {
        buf.reserve(100000);
        if (findAttribute("Values"s, &buf) == nullptr) return lastErrorPtr();
        int numValues = GSUtil::CountTokens(buf.c_str());
        std::vector<double> values;
        values.reserve(size_t(numValues));
        GSUtil::Double(buf, numValues, values.data());
        if (findAttribute("Durations"s, &buf) == nullptr) return lastErrorPtr();
        int numDurations = GSUtil::CountTokens(buf.c_str());
        if (numDurations != numValues)
        {
            setLastError("StepDriver ID=\""s + name() + "\" number of values ("s + std::to_string(numValues) + ") must match number of durations ("s + std::to_string(numDurations) + ")"s);
            return lastErrorPtr();
        }
        std::vector<double> durations;
        durations.reserve(size_t(numDurations));
        GSUtil::Double(buf, numDurations, durations.data());
        this->SetValuesAndDurations(size_t(numValues), values.data(), durations.data());
    }
    else
    {
        int steps = GSUtil::Int(buf);
        buf.reserve(size_t(steps * 32));
        if (findAttribute("Values"s, &buf) == nullptr) return lastErrorPtr();
        std::vector<double> values;
        values.reserve(size_t(steps));
        GSUtil::Double(buf, steps, values.data());
        if (findAttribute("Durations"s, &buf) == nullptr) return lastErrorPtr();
        std::vector<double> durations;
        durations.reserve(size_t(steps));
        GSUtil::Double(buf, steps, durations.data());
        this->SetValuesAndDurations(size_t(steps), values.data(), durations.data());
    }

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void StepDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    buf.reserve(size_t(m_ListLength) * 32); // should be big enough but it will grow if necessary anyway
    setAttribute("Type"s, "Step"s);
    setAttribute("Steps"s, *GSUtil::ToString(m_ListLength, &buf));
    setAttribute("Durations"s, *GSUtil::ToString(m_DurationList.data(), size_t(m_ListLength), &buf));
    setAttribute("Values"s, *GSUtil::ToString(m_ValueList.data(), size_t(m_ListLength), &buf));
}


