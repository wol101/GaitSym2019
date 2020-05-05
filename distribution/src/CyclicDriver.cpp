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

void CyclicDriver::SetPhaseDelay(double phaseDelay)
{
    m_PhaseDelay = phaseDelay;
}

// Note list is delt0, v0, delt1, v1, delt2, v2 etc
// times are intervals not absolute simulation times
void CyclicDriver::SetValuesAndDurations(int size, double *values, double *durations)
{
    if (size <= 0)
    {
        std::cerr << "CyclicDriver::SetValueDurationPairs error: size = " << size << "\n";
        return;
    }
    m_ListLength = size;
    m_DurationList.resize(size_t(m_ListLength + 1));
    m_ValueList.resize(size_t(m_ListLength + 1));
    m_DurationList[0] = 0;
    for (int i = 0 ; i < m_ListLength; i++)
    {
        m_DurationList[size_t(i) + 1] = durations[size_t(i)] + m_DurationList[size_t(i)]; // fill the list with absolute times
        m_ValueList[size_t(i)] = values[size_t(i)];
    }
    m_ValueList[size_t(m_ListLength)] = m_ValueList[0]; // this lets the interp version wrap properly
}

void CyclicDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());

    // account for phase
    // m_PhaseDelay is a relative value (0 to 1) but the time offset needs to be positive
    double timeOffset = m_DurationList[size_t(m_ListLength)] - m_DurationList[size_t(m_ListLength)] * m_PhaseDelay;
    double time = simulation()->GetTime() + timeOffset;

    double rem = fmod(time, m_DurationList[size_t(m_ListLength)]);

    if (Interp() == false)
    {
        // optimisation because most of the time this is called it just returns the value
        // used previously
        if (m_DurationList[size_t(m_LastIndex)] <= rem && m_DurationList[size_t(m_LastIndex) + 1] > rem)
        {
            setValue(m_ValueList[size_t(m_LastIndex)]);
        }
        else
        {
            m_LastIndex = GSUtil::BinarySearchRange<double>(m_DurationList.data(), m_ListLength, rem);

            if (m_LastIndex == -1) m_LastIndex = 0; // fixup for not found errors
            setValue(m_ValueList[size_t(m_LastIndex)]);
        }
    }
    else
    {
        // optimisation because most of the time this is called it just returns the value
        // used previously
        if (m_DurationList[size_t(m_LastIndex)] <= rem && m_DurationList[size_t(m_LastIndex) + 1] > rem)
        {
            setValue(((rem - m_DurationList[size_t(m_LastIndex)]) / (m_DurationList[size_t(m_LastIndex) + 1] - m_DurationList[size_t(m_LastIndex)])) *
                    (m_ValueList[size_t(m_LastIndex) + 1] - m_ValueList[size_t(m_LastIndex)]) + m_ValueList[size_t(m_LastIndex)]);
        }
        else
        {
            m_LastIndex = GSUtil::BinarySearchRange<double>(m_DurationList.data(), m_ListLength, rem);

            if (m_LastIndex == -1) m_LastIndex = 0; // fixup for not found errors
            setValue(((rem - m_DurationList[size_t(m_LastIndex)]) / (m_DurationList[size_t(m_LastIndex) + 1] - m_DurationList[size_t(m_LastIndex)])) *
                    (m_ValueList[size_t(m_LastIndex) + 1] - m_ValueList[size_t(m_LastIndex)]) + m_ValueList[size_t(m_LastIndex)]);
        }
    }
}

double CyclicDriver::GetCycleTime()
{
    return m_DurationList[size_t(m_ListLength)];
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *CyclicDriver::createFromAttributes()
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
            setLastError("CyclicDriver ID=\""s + name() + "\" number of values ("s + std::to_string(numValues) + ") must match number of durations ("s + std::to_string(numDurations) + ")"s);
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
        if (findAttribute("Values"s, &buf) == nullptr) return lastErrorPtr();
        std::vector<double> values;
        values.reserve(size_t(steps));
        GSUtil::Double(buf, steps, values.data());
        if (findAttribute("Durations"s, &buf) == nullptr) return lastErrorPtr();
        std::vector<double> durations;
        durations.reserve(size_t(steps));
        GSUtil::Double(buf, steps, durations.data());
        this->SetValuesAndDurations(steps, values.data(), durations.data());
    }

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void CyclicDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    buf.reserve(size_t(m_ListLength) * 32); // should be big enough but it will grow if necessary anyway
    setAttribute("Type"s, "Cyclic"s);
    setAttribute("Steps"s, *GSUtil::ToString(m_ListLength, &buf));
    setAttribute("PhaseDelay"s, *GSUtil::ToString(m_PhaseDelay, &buf));
    setAttribute("Durations"s, *GSUtil::ToString(m_DurationList.data(), size_t(m_ListLength), &buf));
    setAttribute("Values"s, *GSUtil::ToString(m_ValueList.data(), size_t(m_ListLength), &buf));
}

