/*
 *  Driver.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue Aug 04 2009.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 * Virtual class that all drivers descend from
 */

#include "Driver.h"
#include "Drivable.h"
#include "Muscle.h"
#include "Controller.h"
#include "Simulation.h"
#include "PGDMath.h"
#include "GSUtil.h"

#include "pystring.h"

using namespace std::string_literals;

Driver::Driver()
{
}

Driver::~Driver()
{
}

Drivable *Driver::GetTarget(const std::string &name)
{
    auto it = m_targetList.find(name);
    if (it == m_targetList.end()) return nullptr;
    else return it->second;
}



void Driver::AddTarget(Drivable *target)
{
    if (target == nullptr)
    {
        std::cerr << "Error setting target for DRIVER " << GetName() << "\n";
        exit(1);
    }
    m_targetList[target->GetName()] = target;
    target->AddDriver(this);
}

double Driver::Clamp(double value)
{
  return value < m_minValue ? m_minValue : (value > m_maxValue ? m_maxValue : value);
}


void Driver::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == nullptr)
        {
            if (GetName().size() == 0) std::cerr << "NamedObject::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tValue\n";
        }
    }


    if (dumpStream())
    {
        *dumpStream() << simulation()->GetTime() << "\t" << GetValue(simulation()->GetTime()) <<
                "\n";
    }
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Driver::CreateFromAttributes()
{
    if (NamedObject::CreateFromAttributes()) return lastErrorPtr();

    std::string buf;
    double doubleList[2];

    if (GetAttribute("TargetIDList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetNames;
    pystring::split(buf, targetNames);
    if (targetNames.size() == 0)
    {
        setLastError("Driver ID=\""s + GetName() +"\" No targets found in TargetIDList"s);
        return lastErrorPtr();
    }
    m_targetList.clear();
    for (size_t i = 0; i < targetNames.size(); i++)
    {
        auto muscleIter = simulation()->GetMuscleList()->find(targetNames[i]);
        if (muscleIter != simulation()->GetMuscleList()->end()) this->AddTarget(muscleIter->second);
        else
        {
            auto controllerIter = simulation()->GetControllerList()->find(targetNames[i]);
            if (controllerIter != simulation()->GetControllerList()->end()) this->AddTarget(controllerIter->second);
            else
            {
                setLastError("Driver ID=\""s + GetName() +"\" TargetID=\""s + buf + "\" not found"s);
                return lastErrorPtr();
            }
        }
    }

    if (GetAttribute("DriverRange"s, &buf))
    {
        GSUtil::Double(buf, 2, doubleList);
        this->setMinValue(doubleList[0]);
        this->setMaxValue(doubleList[1]);
    }

    if (GetAttribute("LinearInterpolation"s, &buf))
    {
        this->setInterp(GSUtil::Bool(buf));
    }

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Driver::SaveToAttributes()
{
    this->setTag("DRIVER"s);
    this->AppendToAttributes();
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void Driver::AppendToAttributes()
{
    NamedObject::AppendToAttributes();
    std::string buf;
    std::vector<std::string> stringList;
    stringList.reserve(m_targetList.size());
    for (auto it = m_targetList.begin(); it != m_targetList.end(); it++) stringList.push_back(it->first);
    setAttribute("TargetIDList"s, pystring::join(" "s, stringList));
    double doubleList[2] = { m_minValue, m_maxValue };
    setAttribute("DriverRange"s, *GSUtil::ToString(doubleList, 2, &buf));
    setAttribute("LinearInterpolation"s, *GSUtil::ToString(Interp(), &buf));
}

double Driver::MinValue() const
{
    return m_minValue;
}

void Driver::setMinValue(double MinValue)
{
    m_minValue = MinValue;
}

double Driver::MaxValue() const
{
    return m_maxValue;
}

void Driver::setMaxValue(double MaxValue)
{
    m_maxValue = MaxValue;
}

double Driver::LastTime() const
{
    return m_lastTime;
}

void Driver::setLastTime(double LastTime)
{
    m_lastTime = LastTime;
}

double Driver::LastValue() const
{
    return m_lastValue;
}

void Driver::setLastValue(double LastValue)
{
    m_lastValue = LastValue;
}

bool Driver::Interp() const
{
    return m_interp;
}

void Driver::setInterp(bool Interp)
{
    m_interp = Interp;
}

const std::map<std::string, Drivable *> *Driver::targetList() const
{
    return &m_targetList;
}

