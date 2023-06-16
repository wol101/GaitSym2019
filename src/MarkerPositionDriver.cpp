/*
 *  MarkerPositionDriver.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 11th May 2022.
 *  Copyright (c) 2022 Bill Sellers. All rights reserved.
 *
 *  Sets the values of the marker position at specified times
 *
 */

#include "MarkerPositionDriver.h"
#include "GSUtil.h"
#include "Simulation.h"
#include "Marker.h"

#include "pystring.h"

#include <algorithm>
#include <limits>

using namespace std::string_literals;

MarkerPositionDriver::MarkerPositionDriver()
{
}

MarkerPositionDriver::~MarkerPositionDriver()
{
}

void MarkerPositionDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());

    double time = simulation()->GetTime();

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
    if (m_index >= m_xPositionList.size())
        m_index = m_xPositionList.size() - 1;

    double x, y, z;
    if (Interp() == false)
    {
        x = m_xPositionList[m_index];
        y = m_yPositionList[m_index];
        z = m_zPositionList[m_index];
    }
    else
    {
        if (m_index < m_xPositionList.size() - 1)
        {
            x = ((time - m_changeTimes[m_index]) / (m_changeTimes[m_index + 1] - m_changeTimes[m_index])) * (m_xPositionList[m_index + 1] - m_xPositionList[m_index]) + m_xPositionList[m_index];
            y = ((time - m_changeTimes[m_index]) / (m_changeTimes[m_index + 1] - m_changeTimes[m_index])) * (m_yPositionList[m_index + 1] - m_yPositionList[m_index]) + m_yPositionList[m_index];
            z = ((time - m_changeTimes[m_index]) / (m_changeTimes[m_index + 1] - m_changeTimes[m_index])) * (m_zPositionList[m_index + 1] - m_zPositionList[m_index]) + m_zPositionList[m_index];
        }
        else
        {
            x = m_xPositionList.back();
            y = m_yPositionList.back();
            z = m_zPositionList.back();
        }
    }
    if (m_referenceMarker == nullptr) // world marker
    {
        for (auto &&it : m_targetMarkerList)
        {
            it->SetWorldPosition(x, y, z);
        }
    }
    else
    {
        for (auto &&it : m_targetMarkerList)
        {
            pgd::Vector3 p = m_referenceMarker->GetWorldPosition(pgd::Vector3(x, y, z));
            it->SetWorldPosition(p.x, p.y, p.z);
        }
    }
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *MarkerPositionDriver::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    buf.reserve(100000);
    if (findAttribute("Times"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<double> times;
    GSUtil::Double(buf, &times);
    if (times.size() == 0 || times[0] != 0)
    {
        setLastError("Driver ID=\""s + name() +"\" Times[0] must equal zero"s);
        return lastErrorPtr();
    }
    if (findAttribute("XPositions"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<double> xPositions;
    GSUtil::Double(buf, &xPositions);
    if (findAttribute("YPositions"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<double> yPositions;
    GSUtil::Double(buf, &yPositions);
    if (findAttribute("ZPositions"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<double> zPositions;
    GSUtil::Double(buf, &zPositions);

    if (times.size() != xPositions.size() || times.size() != yPositions.size() || times.size() != zPositions.size())
    {
        setLastError("MarkerPositionDriver ID=\""s + name() + "\" number of times ("s + std::to_string(times.size()) + ") must match number of positions ("s
                     + std::to_string(xPositions.size()) + ","s  + std::to_string(yPositions.size()) + ","s  + std::to_string(zPositions.size()) + ")"s);
        return lastErrorPtr();
    }
    m_changeTimes = std::move(times);
    m_xPositionList = std::move(xPositions);
    m_yPositionList = std::move(yPositions);
    m_zPositionList = std::move(zPositions);
    m_changeTimes.push_back(std::numeric_limits<double>::max()); // needed because of the way we test for ranges

    if (findAttribute("MarkerIDList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetNames;
    pystring::split(buf, targetNames);
    if (targetNames.size() == 0)
    {
        setLastError("Driver ID=\""s + name() +"\" TargetIDList is empty"s);
        return lastErrorPtr();
    }
    m_targetMarkerList.clear();
    std::vector<NamedObject *> upstreamObjects;
    upstreamObjects.reserve(targetNames.size());
    for (size_t i = 0; i < targetNames.size(); i++)
    {
        auto markerIter = simulation()->GetMarkerList()->find(targetNames[i]);
        if (markerIter != simulation()->GetMarkerList()->end())
        {
            m_targetMarkerList.push_back(markerIter->second.get());
            upstreamObjects.push_back(markerIter->second.get());
        }
        else
        {
            setLastError("Driver ID=\""s + name() +"\" MarkerID=\""s + buf + "\" not found"s);
            return lastErrorPtr();
        }
    }
    if (findAttribute("ReferenceMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    if (buf == "World"s)
    {
        m_referenceMarker = nullptr;
    }
    else
    {
        Marker *marker = simulation()->GetMarker(buf);
        if (marker) { m_referenceMarker = marker; }
        else
        {
            setLastError("Driver ID=\""s + name() +"\" ReferenceMarkerID=\""s + buf + "\" not found"s);
            return lastErrorPtr();
        }
    }
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void MarkerPositionDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    buf.reserve(m_changeTimes.size() * 32); // should be big enough but it will grow if necessary anyway
    std::vector<std::string> stringList;
    stringList.reserve(m_targetMarkerList.size());
    setAttribute("Type"s, "MarkerPosition"s);
    setAttribute("Times"s, *GSUtil::ToString(m_changeTimes.data(), m_changeTimes.size() - 1, &buf));
    setAttribute("XPositions"s, *GSUtil::ToString(m_xPositionList.data(), m_xPositionList.size(), &buf));
    setAttribute("YPositions"s, *GSUtil::ToString(m_yPositionList.data(), m_yPositionList.size(), &buf));
    setAttribute("ZPositions"s, *GSUtil::ToString(m_zPositionList.data(), m_zPositionList.size(), &buf));
    for (auto &&it : m_targetMarkerList) stringList.push_back(it->name());
    setAttribute("MarkerIDList"s, pystring::join(" "s, stringList));
    if (m_referenceMarker) setAttribute("ReferenceMarkerID"s, m_referenceMarker->name());
    else setAttribute("ReferenceMarkerID"s, "World"s);
}


