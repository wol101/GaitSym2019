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

using namespace std::string_literals;

DataTargetQuaternion::DataTargetQuaternion()
{
}

DataTargetQuaternion::~DataTargetQuaternion()
{
}

// note in this case the pointer is to a list of the elements of
// size quaternions
// note quaternion is (qs,qx,qy,qz)
void DataTargetQuaternion::SetTargetValues(int size, double *values)
{
    if (size != TargetTimeListLength())
    {
        std::cerr << "DataTargetQuaternion::SetTargetValues error: size = " << size << "\n";
        return;
    }
    m_QValueListLength = size;
    m_QValueList.resize(size_t(m_QValueListLength));
    for (size_t i = 0 ; i < size_t(m_QValueListLength); i++)
    {
        m_QValueList[i].n = values[i * 4];
        m_QValueList[i].v.x = values[i * 4 + 1];
        m_QValueList[i].v.y = values[i * 4 + 2];
        m_QValueList[i].v.z = values[i * 4 + 3];
        m_QValueList[i].Normalize(); // always do this on input.
    }
}

// returns the degree of match to the stored values
// in this case this is the angle between the two quaternions
double DataTargetQuaternion::GetError(int valueListIndex)
{
    const double *r;
    Body *body;
    Geom *geom;
    dQuaternion q;
    double angle = 0;
    if (valueListIndex < 0) valueListIndex = 0;
    if (valueListIndex >= m_QValueListLength)
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
double DataTargetQuaternion::GetError(double time)
{
    const double *r;
    Body *body;
    Geom *geom;
    dQuaternion q;
    double angle = 0;

    int index = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), time);
    if (index < 0) index = 0;
    if (index >= m_QValueListLength - 1)
    {
        std::cerr << "Warning: DataTargetVector::GetMatchValue index out of range\n";
        return 0;
    }
    int indexNext = index + 1;

    // do a slerp interpolation between the target quaternions
    double interpolationFraction = (time - TargetTimeList()[size_t(index)]) / (TargetTimeList()[size_t(indexNext)] - TargetTimeList()[size_t(index)]);
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

std::string DataTargetQuaternion::dump()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (getFirstDump())
    {
        setFirstDump(false);
        ss << "Time\tTargetQW\tTargetQX\tTargetQY\tTargetQZ\tActualQW\tActualQX\tActualQY\tActualQZ\tAngle\n";
    }
    Body *body;
    Geom *geom;
    const double *r;
    double angle = 0;
    dQuaternion q;

    int valueListIndex = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), simulation()->GetTime());
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

    ss << simulation()->GetTime() <<
          "\t" << m_QValueList[size_t(valueListIndex)].n << "\t" << m_QValueList[size_t(valueListIndex)].v.x << "\t" << m_QValueList[size_t(valueListIndex)].v.y << "\t" << m_QValueList[size_t(valueListIndex)].v.z <<
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
    if (int(targetValuesTokens.size()) != TargetTimeListLength() * 4)
    {
        setLastError("DataTargetQuaternion ID=\""s + name() +"\" Number of values in TargetValues does not match 4 * TargetTimes"s);
        return lastErrorPtr();
    }
    std::vector<double> targetValues;
    targetValues.reserve(targetValuesTokens.size());
    for (auto token : targetValuesTokens) targetValues.push_back(GSUtil::Double(token));
    SetTargetValues(int(targetValues.size()) / 4, targetValues.data());


    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void DataTargetQuaternion::appendToAttributes()
{
    DataTarget::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Scalar"s);
    std::vector<double> valueList;
    valueList.reserve(size_t(m_QValueListLength) * 4);
    for (size_t i = 0; i < size_t(m_QValueListLength); i++)
    {
        valueList.push_back(m_QValueList[i].n);
        valueList.push_back(m_QValueList[i].v.x);
        valueList.push_back(m_QValueList[i].v.y);
        valueList.push_back(m_QValueList[i].v.z);
    }
    setAttribute("TargetValues"s, *GSUtil::ToString(valueList.data(), valueList.size(), &buf));
    setAttribute("TargetID"s, m_Target->name());
}

