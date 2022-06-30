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

#include <sstream>

using namespace std::string_literals;

Driver::Driver()
{
}

Driver::~Driver()
{
}


Drivable *Driver::GetTarget(const std::string &name)
{
    if (name.size() == 0)
    {
        if (m_targetList.size()) return m_targetList.begin()->second;
        else return nullptr;
    }
    auto it = m_targetList.find(name);
    if (it == m_targetList.end()) return nullptr;
    else return it->second;
}

void Driver::SendData()
{
    for (auto &&it : m_targetList)
    {
        it.second->ReceiveData(Clamp(m_value), simulation()->GetStepCount());
    }
}

int Driver::AddTarget(Drivable *target)
{
    if (target == nullptr) return __LINE__;
    NamedObject *object = dynamic_cast<NamedObject *>(target);
    if (object == nullptr) return __LINE__;
    m_targetList[object->name()] = target;
    return 0;
}

double Driver::Clamp(double value)
{
  return value < m_minValue ? m_minValue : (value > m_maxValue ? m_maxValue : value);
}

std::string Driver::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Name\tTime\tValue\n";
    }
    ss << name() << "\t" << simulation()->GetTime() << "\t" << value() << "\n";
    return ss.str();
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Driver::createFromAttributes()
{
    if (NamedObject::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    double doubleList[2];

    if (findAttribute("TargetIDList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetNames;
    pystring::split(buf, targetNames);
//    if (targetNames.size() == 0)
//    {
//        setLastError("Driver ID=\""s + name() +"\" TargetIDList is empty"s);
//        return lastErrorPtr();
//    }
    m_targetList.clear();
    std::vector<NamedObject *> upstreamObjects;
    upstreamObjects.reserve(targetNames.size());
    for (size_t i = 0; i < targetNames.size(); i++)
    {
        auto muscleIter = simulation()->GetMuscleList()->find(targetNames[i]);
        if (muscleIter != simulation()->GetMuscleList()->end())
        {
            this->AddTarget(muscleIter->second.get());
            upstreamObjects.push_back(muscleIter->second.get());
        }
        else
        {
            auto controllerIter = simulation()->GetControllerList()->find(targetNames[i]);
            if (controllerIter != simulation()->GetControllerList()->end())
            {
                this->AddTarget(controllerIter->second.get());
                upstreamObjects.push_back(controllerIter->second.get());
            }
            else
            {
                setLastError("Driver ID=\""s + name() +"\" TargetID=\""s + buf + "\" not found"s);
                return lastErrorPtr();
            }
        }
    }

    if (findAttribute("DriverRange"s, &buf))
    {
        GSUtil::Double(buf, 2, doubleList);
        this->setMinValue(doubleList[0]);
        this->setMaxValue(doubleList[1]);
    }

    if (findAttribute("LinearInterpolation"s, &buf))
    {
        this->setInterp(GSUtil::Bool(buf));
    }

    setUpstreamObjects(std::move(upstreamObjects));
    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Driver::saveToAttributes()
{
    this->setTag("DRIVER"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void Driver::appendToAttributes()
{
    NamedObject::appendToAttributes();
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

int64_t Driver::lastStepCount() const
{
    return m_lastStepCount;
}

void Driver::setLastStepCount(const int64_t &lastStepCount)
{
    m_lastStepCount = lastStepCount;
}

double Driver::value() const
{
    return m_value;
}

void Driver::setValue(double value)
{
    m_value = value;
}

