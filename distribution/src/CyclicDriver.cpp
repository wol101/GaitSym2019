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



#include <algorithm>

using namespace std::string_literals;

CyclicDriver::CyclicDriver()
{
}

CyclicDriver::~CyclicDriver()
{
    if (m_DurationList) delete [] m_DurationList;
    if (m_ValueList) delete [] m_ValueList;
}

void CyclicDriver::SetPhaseDelay(double phaseDelay)
{
    m_PhaseDelay = phaseDelay;
}

// Note list is delt0, v0, delt1, v1, delt2, v2 etc
// times are intervals not absolute simulation times
void CyclicDriver::SetValuesAndDurations(int size, double *values, double *durations)
{
    int i;
    if (size <= 0)
    {
        std::cerr << "CyclicDriver::SetValueDurationPairs error: size = " << size << "\n";
        return;
    }
    if (m_ListLength != size)
    {
        if (m_DurationList) delete [] m_DurationList;
        if (m_ValueList) delete [] m_ValueList;
        m_ListLength = size;
        m_DurationList = new double[size_t(m_ListLength + 1)]; // this is +1 because the first entry is zero
        m_ValueList = new double[size_t(m_ListLength + 1)]; // this lets the interp version wrap properly
    }
    m_DurationList[0] = 0;
    for (i = 0 ; i < m_ListLength; i++)
    {
        m_DurationList[i + 1] = durations[i] + m_DurationList[i]; // fill the list with absolute times
        m_ValueList[i] = values[i];
    }
    m_ValueList[m_ListLength] = m_ValueList[0]; // this lets the interp version wrap properly
}

double CyclicDriver::GetValue(double time)
{
    if (time == LastTime()) return LastValue();
    setLastTime(time);

    double v;
    // account for phase
    // m_PhaseDelay is a relative value (0 to 1) but the time offset needs to be positive
    double timeOffset = m_DurationList[m_ListLength] - m_DurationList[m_ListLength] * m_PhaseDelay;
    time = time + timeOffset;

    double rem = fmod(time, m_DurationList[m_ListLength]);

    if (Interp() == false)
    {
        // optimisation because most of the time this is called it just returns the value
        // used previously
        if (m_DurationList[m_LastIndex] <= rem && m_DurationList[m_LastIndex + 1] > rem)
        {
            v = m_ValueList[m_LastIndex];
        }
        else
        {
            m_LastIndex = GSUtil::BinarySearchRange<double>(m_DurationList, m_ListLength, rem);

            if (m_LastIndex == -1) m_LastIndex = 0; // fixup for not found errors
            v = m_ValueList[m_LastIndex];
        }
    }
    else
    {
        // optimisation because most of the time this is called it just returns the value
        // used previously
        if (m_DurationList[m_LastIndex] <= rem && m_DurationList[m_LastIndex + 1] > rem)
        {
            v = ((rem - m_DurationList[m_LastIndex]) / (m_DurationList[m_LastIndex + 1] - m_DurationList[m_LastIndex])) *
                    (m_ValueList[m_LastIndex + 1] - m_ValueList[m_LastIndex]) + m_ValueList[m_LastIndex];
        }
        else
        {
            m_LastIndex = GSUtil::BinarySearchRange<double>(m_DurationList, m_ListLength, rem);

            if (m_LastIndex == -1) m_LastIndex = 0; // fixup for not found errors
            v = ((rem - m_DurationList[m_LastIndex]) / (m_DurationList[m_LastIndex + 1] - m_DurationList[m_LastIndex])) *
                    (m_ValueList[m_LastIndex + 1] - m_ValueList[m_LastIndex]) + m_ValueList[m_LastIndex];
        }
    }


    setLastValue(Clamp(v));
    return LastValue();
}

double CyclicDriver::GetCycleTime()
{
    return m_DurationList[m_ListLength];
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *CyclicDriver::CreateFromAttributes()
{
    if (Driver::CreateFromAttributes()) return lastErrorPtr();

    std::string buf;
    if (GetAttribute("Steps"s, &buf) == nullptr)
    {
        buf.reserve(100000);
        if (GetAttribute("Values"s, &buf) == nullptr) return lastErrorPtr();
        int numValues = GSUtil::CountTokens(buf.c_str());
        std::vector<double> values;
        values.reserve(size_t(numValues));
        GSUtil::Double(buf, numValues, values.data());
        if (GetAttribute("Durations"s, &buf) == nullptr) return lastErrorPtr();
        int numDurations = GSUtil::CountTokens(buf.c_str());
        if (numDurations != numValues)
        {
            setLastError("CyclicDriver ID=\""s + GetName() + "\" number of values ("s + std::to_string(numValues) + ") must match number of durations ("s + std::to_string(numDurations) + ")"s);
            return lastErrorPtr();
        }
        std::vector<double> durations;
        durations.reserve(size_t(numDurations));
        GSUtil::Double(buf, numDurations, durations.data());
        this->SetValuesAndDurations(numValues, values.data(), durations.data());
    }
    else
    {
        int steps = GSUtil::Int(buf);
        buf.reserve(size_t(steps * 32));
        if (GetAttribute("Values"s, &buf) == nullptr) return lastErrorPtr();
        std::vector<double> values;
        values.reserve(size_t(steps));
        GSUtil::Double(buf, steps, values.data());
        if (GetAttribute("Durations"s, &buf) == nullptr) return lastErrorPtr();
        std::vector<double> durations;
        durations.reserve(size_t(steps));
        GSUtil::Double(buf, steps, durations.data());
        this->SetValuesAndDurations(steps, values.data(), durations.data());
    }

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void CyclicDriver::AppendToAttributes()
{
    Driver::AppendToAttributes();
    std::string buf;
    buf.reserve(size_t(m_ListLength) * 32); // should be big enough but it will grow if necessary anyway
    setAttribute("Type"s, "Cyclic"s);
    setAttribute("Steps"s, *GSUtil::ToString(m_ListLength, &buf));
    setAttribute("PhaseDelay"s, *GSUtil::ToString(m_PhaseDelay, &buf));
    setAttribute("Durations"s, *GSUtil::ToString(m_DurationList, size_t(m_ListLength), &buf));
    setAttribute("Values"s, *GSUtil::ToString(m_ValueList, size_t(m_ListLength), &buf));
}

