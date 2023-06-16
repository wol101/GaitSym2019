/*
 *  DataTargetQuaternion.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
 */

#include "DataTargetQuaternion.h"
#include "Body.h"
#include "PGDMath.h"
#include "GSUtil.h"
#include "DataFile.h"
#include "Geom.h"
#include "Marker.h"

#include "pystring.h"

#include <sstream>
#include <algorithm>

using namespace std::string_literals;

DataTargetQuaternion::DataTargetQuaternion()
{
}

// returns the degree of match to the stored values
// in this case this is the angle between the two quaternions
double DataTargetQuaternion::calculateError(size_t valueListIndex)
{
    const double *r;
    Body *body;
    Geom *geom;
    dQuaternion q;
    double angle = 0;
    if (valueListIndex >= m_QValueList.size())
    {
        std::cerr << "Warning: DataTargetQuaternion::GetMatchValue valueListIndex out of range\n";
        return 0;
    }

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetQuaternion();
        angle = pgd::FindAngle(m_QValueList[size_t(valueListIndex)], pgd::Quaternion(r[0], r[1], r[2], r[3]));
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldQuaternion(q);
        angle = pgd::FindAngle(m_QValueList[size_t(valueListIndex)], pgd::Quaternion(q[0], q[1], q[2], q[3]));
    }
    else
    {
        std::cerr << "DataTargetQuaternion target missing error " << name() << "\n";
    }
    return angle;
}

// returns the degree of match to the stored values
// in this case this is the angle between the two quaternions
double DataTargetQuaternion::calculateError(double time)
{
    const double *r;
    Body *body;
    Geom *geom;
    dQuaternion q;
    double angle = 0;

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

    // do a slerp interpolation between the target quaternions
    double delTime = (*targetTimeList())[size_t(indexNext)] - (*targetTimeList())[size_t(index)];
    double interpolationFraction;
    if (delTime < DBL_EPSILON) interpolationFraction = 0;
    else interpolationFraction = (time - (*targetTimeList())[size_t(index)]) / delTime;
    pgd::Quaternion interpolatedTarget = pgd::slerp(m_QValueList[size_t(index)], m_QValueList[size_t(indexNext)], interpolationFraction);

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetQuaternion();
        angle = pgd::FindAngle(interpolatedTarget, pgd::Quaternion(r[0], r[1], r[2], r[3]));
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldQuaternion(q);
        angle = pgd::FindAngle(interpolatedTarget, pgd::Quaternion(q[0], q[1], q[2], q[3]));
    }
    else
    {
        std::cerr << "DataTargetQuaternion target missing error " << name() << "\n";
    }
    return angle;
}

std::string DataTargetQuaternion::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tTargetQW\tTargetQX\tTargetQY\tTargetQZ\tActualQW\tActualQX\tActualQY\tActualQZ\tAngle\n";
    }
    Body *body;
    Geom *geom;
    const double *r;
    double angle = 0;
    dQuaternion q = {0, 0, 0, 0};

    size_t valueListIndex = 0;
    auto lowerBounds = std::lower_bound(targetTimeList()->begin(), targetTimeList()->end(), simulation()->GetTime());
    if (lowerBounds != targetTimeList()->end()) valueListIndex = std::distance(targetTimeList()->begin(), lowerBounds);

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetQuaternion();
        angle = pgd::FindAngle(m_QValueList[size_t(valueListIndex)], pgd::Quaternion(r[0], r[1], r[2], r[3]));
        q[0] = r[0];
        q[1] = r[1];
        q[2] = r[2];
        q[3] = r[3];
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldQuaternion(q);
        angle = pgd::FindAngle(m_QValueList[size_t(valueListIndex)], pgd::Quaternion(q[0], q[1], q[2], q[3]));
    }

    ss << simulation()->GetTime() <<
          "\t" << m_QValueList[size_t(valueListIndex)].n << "\t" << m_QValueList[size_t(valueListIndex)].x << "\t" << m_QValueList[size_t(valueListIndex)].y << "\t" << m_QValueList[size_t(valueListIndex)].z <<
          "\t" << q[0] << "\t" << q[1] << "\t" << q[2] << "\t" << q[3] <<
          "\t" << angle <<
          "\n";
    return ss.str();
}

void DataTargetQuaternion::SetTarget(NamedObject *target)
{
    m_Target = target;
}

NamedObject *DataTargetQuaternion::GetTarget()
{
    return m_Target;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *DataTargetQuaternion::createFromAttributes()
{
    if (DataTarget::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    if (findAttribute("DataType"s, &buf) == nullptr) return lastErrorPtr();
    m_Target = nullptr;
    if (findAttribute("TargetID"s, &buf) == nullptr) return lastErrorPtr();
    for (bool once = true; once; once = false)
    {
        auto iterBody = simulation()->GetBodyList()->find(buf);
        if (iterBody != simulation()->GetBodyList()->end()) { m_Target = iterBody->second.get(); break; }
        auto iterGeom = simulation()->GetGeomList()->find(buf);
        if (iterGeom != simulation()->GetGeomList()->end()) { m_Target = iterGeom->second.get(); break; }
        auto iterMarker = simulation()->GetMarkerList()->find(buf);
        if (iterMarker != simulation()->GetMarkerList()->end()) { m_Target = iterMarker->second.get(); break; }
    }
    if (!m_Target)
    {
        setLastError("DataTargetQuaternion ID=\""s + name() +"\" TargetID not found "s + buf);
        return lastErrorPtr();
    }

    if (findAttribute("TargetValues"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetValuesTokens;
    pystring::split(buf, targetValuesTokens);
    if (targetValuesTokens.size() == 0)
    {
        setLastError("DataTargetQuaternion ID=\""s + name() +"\" No values found in TargetValues"s);
        return lastErrorPtr();
    }
    if (targetValuesTokens.size() != targetTimeList()->size() * 4)
    {
        setLastError("DataTargetQuaternion ID=\""s + name() +"\" Number of values in TargetValues does not match 4 * TargetTimes"s);
        return lastErrorPtr();
    }
    m_QValueList.clear();
    m_QValueList.reserve(targetTimeList()->size());
    for (size_t i = 0; i < targetTimeList()->size(); i++)
    {
        pgd::Quaternion q(GSUtil::Double(targetValuesTokens[i * 4]), GSUtil::Double(targetValuesTokens[i * 4 + 1]),
                          GSUtil::Double(targetValuesTokens[i * 4 + 2]), GSUtil::Double(targetValuesTokens[i * 4 + 3]));
        m_QValueList.push_back(q);
    }

    if (m_Target) setUpstreamObjects({m_Target});
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void DataTargetQuaternion::appendToAttributes()
{
    DataTarget::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Scalar"s);
    std::vector<double> valueList;
    valueList.reserve(m_QValueList.size() * 4);
    for (size_t i = 0; i < m_QValueList.size(); i++)
    {
        valueList.push_back(m_QValueList[i].n);
        valueList.push_back(m_QValueList[i].x);
        valueList.push_back(m_QValueList[i].y);
        valueList.push_back(m_QValueList[i].z);
    }
    setAttribute("TargetValues"s, *GSUtil::ToString(valueList.data(), valueList.size(), &buf));
    setAttribute("TargetID"s, m_Target->name());
}

