/*
 *  DataTargetMarkerCompare.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Thu Dec 24 2020.
 *  Copyright (c) 2020 Bill Sellers. All rights reserved.
 *
 */

#include "DataTargetMarkerCompare.h"
#include "Marker.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "PGDMath.h"

#include "pystring.h"

#include <sstream>
#include <algorithm>

using namespace std::string_literals;

DataTargetMarkerCompare::DataTargetMarkerCompare()
{

}

double DataTargetMarkerCompare::calculateError(double time)
{
    m_errorScore = 0;

    size_t index, indexNext;
    auto lowerBound = std::lower_bound(targetTimeList()->begin(), targetTimeList()->end(), time);
    auto upperBound = std::upper_bound(targetTimeList()->begin(), targetTimeList()->end(), time);
    if (lowerBound == targetTimeList()->begin()) // time <= lowest value in the list
    {
        index = 0;
        indexNext = index;
    }
    else if (upperBound == targetTimeList()->end()) // time > highest value in the list
    {
        index = targetTimeList()->size() - 1;
        indexNext = index;
    }
    else
    {
        index = std::distance(targetTimeList()->begin(), lowerBound) - 1; // subtracting 1 because lower bound gives an index 1 higher than expected (IMO)
        indexNext = std::min(index + 1, targetTimeList()->size() - 1);
    }

    while (true)
    {
        if (m_marker1Comparison == XWP && m_marker2Comparison == XWP)
        {
            double distance = m_marker2->GetWorldPosition().x - m_marker1->GetWorldPosition().x;
            m_errorScore = (distance - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        }
        if (m_marker1Comparison == YWP && m_marker2Comparison == YWP)
        {
            double distance = m_marker2->GetWorldPosition().y - m_marker1->GetWorldPosition().y;
            m_errorScore = (distance - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        }
        if (m_marker1Comparison == ZWP && m_marker2Comparison == ZWP)
        {
            double distance = m_marker2->GetWorldPosition().z - m_marker1->GetWorldPosition().z;
            m_errorScore = (distance - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        }
        if (m_marker1Comparison == Distance && m_marker2Comparison == Distance)
        {
            double distance = (m_marker1->GetWorldPosition() - m_marker2->GetWorldPosition()).Magnitude();
            m_errorScore = (distance - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        }
        if (m_marker1Comparison == Angle && m_marker2Comparison == Angle)
        {
            pgd::Quaternion q = pgd::FindRotation(m_marker1->GetWorldQuaternion(), m_marker2->GetWorldQuaternion());
            double angle = pgd::QGetAngle(q);
            m_errorScore = (angle - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        }
        if (m_marker1Comparison == LinearVelocity && m_marker2Comparison == LinearVelocity)
        {
            double linearVelocity = (m_marker1->GetWorldLinearVelocity() - m_marker2->GetWorldLinearVelocity()).Magnitude();
            m_errorScore = (linearVelocity - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        }
        if (m_marker1Comparison == AngularVelocity && m_marker2Comparison == AngularVelocity)
        {
            double angularVelocity = (m_marker1->GetWorldAngularVelocity() - m_marker2->GetWorldAngularVelocity()).Magnitude();
            m_errorScore = (angularVelocity - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        }
        pgd::Vector3 axis1, axis2;
        while (true)
        {
            if (m_marker1Comparison == XAD) { axis1 = m_marker1->GetWorldAxis(Marker::X); break; }
            if (m_marker1Comparison == YAD) { axis1 = m_marker1->GetWorldAxis(Marker::Y); break; }
            if (m_marker1Comparison == ZAD) { axis1 = m_marker1->GetWorldAxis(Marker::Z); break; }
            break;
        }
        while (true)
        {
            if (m_marker2Comparison == XAD) { axis2 = m_marker2->GetWorldAxis(Marker::X); break; }
            if (m_marker2Comparison == YAD) { axis2 = m_marker2->GetWorldAxis(Marker::Y); break; }
            if (m_marker2Comparison == ZAD) { axis2 = m_marker2->GetWorldAxis(Marker::Z); break; }
            break;
        }
        // for two vectors
        // angle = acos(v1 dot v2)
        // axis = norm(v1 cross v2)
        double angle = std::acos(axis1 * axis2);
        m_errorScore = (angle - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
        break;
    }

    return m_errorScore;
}

double DataTargetMarkerCompare::calculateError(size_t index)
{
    m_errorScore = 0;

    if (index >= m_ValueList.size())
    {
        std::cerr << "Warning: DataTargetScalar::GetMatchValue index out of range\n";
        return 0;
    }

    while (true)
    {
        if (m_marker1Comparison == XWP && m_marker2Comparison == XWP)
        {
            double distance = m_marker2->GetWorldPosition().x - m_marker1->GetWorldPosition().x;
            m_errorScore = (distance - m_ValueList[size_t(index)]);
            break;
        }
        if (m_marker1Comparison == YWP && m_marker2Comparison == YWP)
        {
            double distance = m_marker2->GetWorldPosition().y - m_marker1->GetWorldPosition().y;
            m_errorScore = (distance - m_ValueList[size_t(index)]);
            break;
        }
        if (m_marker1Comparison == ZWP && m_marker2Comparison == ZWP)
        {
            double distance = m_marker2->GetWorldPosition().z - m_marker1->GetWorldPosition().z;
            m_errorScore = (distance - m_ValueList[size_t(index)]);
            break;
        }
        if (m_marker1Comparison == Distance && m_marker2Comparison == Distance)
        {
            double distance = (m_marker1->GetWorldPosition() - m_marker2->GetWorldPosition()).Magnitude();
            m_errorScore = (distance - m_ValueList[size_t(index)]);
            break;
        }
        if (m_marker1Comparison == Angle && m_marker2Comparison == Angle)
        {
            double angle = pgd::FindAngle(m_marker1->GetWorldQuaternion(), m_marker2->GetWorldQuaternion());
            m_errorScore = (angle - m_ValueList[size_t(index)]);
            break;
        }
        if (m_marker1Comparison == LinearVelocity && m_marker2Comparison == LinearVelocity)
        {
            double linearVelocity = (m_marker1->GetWorldLinearVelocity() - m_marker2->GetWorldLinearVelocity()).Magnitude();
            m_errorScore = (linearVelocity - m_ValueList[size_t(index)]);
            break;
        }
        if (m_marker1Comparison == AngularVelocity && m_marker2Comparison == AngularVelocity)
        {
            double angularVelocity = (m_marker1->GetWorldAngularVelocity() - m_marker2->GetWorldAngularVelocity()).Magnitude();
            m_errorScore = (angularVelocity - m_ValueList[size_t(index)]);
            break;
        }
        pgd::Vector3 axis1, axis2;
        switch (m_marker1Comparison)
        {
        case XAD:
            axis1 = m_marker1->GetWorldAxis(Marker::X);
            break;
        case YAD:
            axis1 = m_marker1->GetWorldAxis(Marker::Y);
            break;
        case ZAD:
            axis1 = m_marker1->GetWorldAxis(Marker::Z);
            break;
        default:
            break;
        }
        switch (m_marker2Comparison)
        {
        case XAD:
            axis2 = m_marker2->GetWorldAxis(Marker::X);
            break;
        case YAD:
            axis2 = m_marker2->GetWorldAxis(Marker::Y);
            break;
        case ZAD:
            axis2 = m_marker2->GetWorldAxis(Marker::Z);
            break;
        default:
            break;
        }
        // for two vectors
        // angle = acos(v1 dot v2)
        // axis = norm(v1 cross v2)
        double angle = std::acos(axis1 * axis2);
        m_errorScore = (angle - m_ValueList[size_t(index)]);
        break;
    }

     return m_errorScore;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *DataTargetMarkerCompare::createFromAttributes()
{
    if (DataTarget::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    if (findAttribute("Marker1ID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *marker1 = simulation()->GetMarker(buf);
    if (!marker1)
    {
        setLastError("DataTargetMarkerCompare ID=\""s + name() +"\" Marker1 not found "s + buf);
        return lastErrorPtr();
    }
    if (findAttribute("Marker2ID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *marker2 = simulation()->GetMarker(buf);
    if (!marker2)
    {
        setLastError("DataTargetMarkerCompare ID=\""s + name() +"\" Marker2 not found "s + buf);
        return lastErrorPtr();
    }
    m_marker1 = marker1;
    m_marker2 = marker2;

    if (findAttribute("Marker1Comparison"s, &buf) == nullptr) return lastErrorPtr();
    size_t comparisonIndex;
    for (comparisonIndex = 0; comparisonIndex < comparisonCount; comparisonIndex++)
    {
        if (buf == comparisonStrings(comparisonIndex))
        {
            m_marker1Comparison = static_cast<Comparison>(comparisonIndex);
            break;
        }
    }
    if (comparisonIndex >= comparisonCount)
    {
        setLastError("DataTargetMarkerCompare ID=\""s + name() +"\" Unrecognised Marker1Comparison "s + buf);
        return lastErrorPtr();
    }
    if (findAttribute("Marker2Comparison"s, &buf) == nullptr) return lastErrorPtr();
    for (comparisonIndex = 0; comparisonIndex < comparisonCount; comparisonIndex++)
    {
        if (buf == comparisonStrings(comparisonIndex))
        {
            m_marker2Comparison = static_cast<Comparison>(comparisonIndex);
            break;
        }
    }
    if (comparisonIndex >= comparisonCount)
    {
        setLastError("DataTargetMarkerCompare ID=\""s + name() +"\" Unrecognised Marker2Comparison "s + buf);
        return lastErrorPtr();
    }
    // now check for sensible combinations
    while (true)
    {
        if (m_marker1Comparison == XWP && m_marker2Comparison == XWP) break;
        if (m_marker1Comparison == YWP && m_marker2Comparison == YWP) break;
        if (m_marker1Comparison == ZWP && m_marker2Comparison == ZWP) break;
        if (m_marker1Comparison == Distance && m_marker2Comparison == Distance) break;
        if (m_marker1Comparison == Angle && m_marker2Comparison == Angle) break;
        if (m_marker1Comparison == LinearVelocity && m_marker2Comparison == LinearVelocity) break;
        if (m_marker1Comparison == AngularVelocity && m_marker2Comparison == AngularVelocity) break;
        if ((m_marker1Comparison == XAD || m_marker1Comparison == YAD || m_marker1Comparison == ZAD) &&
            (m_marker2Comparison == XAD || m_marker2Comparison == YAD || m_marker2Comparison == ZAD)) break;
        setLastError("DataTargetMarkerCompare ID=\""s + name() + "\" Marker1Comparison "s + comparisonStrings(m_marker1Comparison) + " not compatible with Marker2Comparison "s + comparisonStrings(m_marker2Comparison));
        return lastErrorPtr();
    }

    if (findAttribute("TargetValues"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetValuesTokens;
    pystring::split(buf, targetValuesTokens);
    if (targetValuesTokens.size() == 0)
    {
        setLastError("DataTarget ID=\""s + name() +"\" No values found in TargetValues"s);
        return lastErrorPtr();
    }
    if (targetValuesTokens.size() != targetTimeList()->size())
    {
        setLastError("DataTargetScalar ID=\""s + name() +"\" Number of values in TargetValues does not match TargetTimes"s);
        return lastErrorPtr();
    }
    m_ValueList.clear();
    m_ValueList.reserve(targetValuesTokens.size());
    for (auto &&token : targetValuesTokens) m_ValueList.push_back(GSUtil::Double(token));

    setUpstreamObjects({m_marker1, m_marker2});
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void DataTargetMarkerCompare::appendToAttributes()
{
    DataTarget::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "MarkerCompare"s);
    setAttribute("Marker1ID"s, m_marker1->name());
    setAttribute("Marker2ID"s, m_marker2->name());
    setAttribute("Marker1Comparison"s, comparisonStrings(m_marker1Comparison));
    setAttribute("Marker2Comparison"s, comparisonStrings(m_marker2Comparison));
    setAttribute("TargetValues"s, *GSUtil::ToString(m_ValueList.data(), m_ValueList.size(), &buf));
}

