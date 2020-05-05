/*
 *  DataTargetVector.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
 */

#include "DataTargetVector.h"
#include "Body.h"
#include "PGDMath.h"
#include "GSUtil.h"
#include "DataFile.h"
#include "HingeJoint.h"
#include "BallJoint.h"
#include "UniversalJoint.h"
#include "Marker.h"
#include "Geom.h"

#include "pystring.h"

#include <sstream>

using namespace std::string_literals;

DataTargetVector::DataTargetVector()
{
}

DataTargetVector::~DataTargetVector()
{
}

// note in this case the pointer is to a list of the elements of
// size vectors
void DataTargetVector::SetTargetValues(int size, double *values)
{
    if (size != TargetTimeListLength())
    {
        std::cerr << "DataTargetVector::SetTargetValues error: size = " << size << "\n";
        return;
    }
    m_VValueListLength = size;
    m_VValueList.resize(size_t(m_VValueListLength));
    for (size_t i = 0 ; i < size_t(m_VValueListLength); i++)
    {
        m_VValueList[i].x = values[i * 3];
        m_VValueList[i].y = values[i * 3 + 1];
        m_VValueList[i].z = values[i * 3 + 2];
    }
}

// returns the degree of match to the stored values
// in this case this is the euclidean distance between the two vectors
double DataTargetVector::GetError(int valueListIndex)
{
    const double *r;
    Body *body;
    Geom *geom;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    Marker *marker;
    double err = 0;
    dVector3 v;
    if (valueListIndex < 0) valueListIndex = 0;
    if (valueListIndex >= m_VValueListLength)
    {
        std::cerr << "Warning: DataTargetVector::GetMatchValue valueListIndex out of range\n";
        return 0;
    }

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetPosition();
        err = (pgd::Vector(r[0], r[1], r[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldPosition(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
   }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
   }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
    }
    else if ((marker = dynamic_cast<Marker *>(GetTarget())) != nullptr)
    {
        pgd::Vector vec = marker->GetWorldPosition();
        err = (vec - m_VValueList[size_t(valueListIndex)]).Magnitude();
    }
    else
    {
        std::cerr << "DataTargetVector target missing error " << name() << "\n";
    }
    return err;
}

// returns the degree of match to the stored values
// in this case this is the euclidean distance between the two vectors
double DataTargetVector::GetError(double time)
{
    const double *r;
    Body *body;
    Geom *geom;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    Marker *marker;
    double err = 0;
    dVector3 v;

    int index = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), time);
    if (index < 0) index = 0;
    if (index >= m_VValueListLength - 1)
    {
        std::cerr << "Warning: DataTargetVector::GetMatchValue index out of range\n";
        return 0;
    }
    int indexNext = index + 1;

    double interpX = GSUtil::Interpolate(TargetTimeList()[size_t(index)], m_VValueList[size_t(index)].x, TargetTimeList()[size_t(indexNext)], m_VValueList[size_t(indexNext)].x, time);
    double interpY = GSUtil::Interpolate(TargetTimeList()[size_t(index)], m_VValueList[size_t(index)].y, TargetTimeList()[size_t(indexNext)], m_VValueList[size_t(indexNext)].y, time);
    double interpZ = GSUtil::Interpolate(TargetTimeList()[size_t(index)], m_VValueList[size_t(index)].z, TargetTimeList()[size_t(indexNext)], m_VValueList[size_t(indexNext)].z, time);
    pgd::Vector interpolatedTarget(interpX, interpY, interpZ);

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetPosition();
        err = (pgd::Vector(r[0], r[1], r[2]) - interpolatedTarget).Magnitude();
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldPosition(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
   }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
   }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
    }
    else if ((marker = dynamic_cast<Marker *>(GetTarget())) != nullptr)
    {
        pgd::Vector vec = marker->GetWorldPosition();
        err = (vec - interpolatedTarget).Magnitude();
    }
    else
    {
        std::cerr << "DataTargetVector target missing error " << name() << "\n";
    }
    return err;
}

std::string DataTargetVector::dump()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (getFirstDump())
    {
        setFirstDump(false);
        ss << "Time\tTargetX\tTargetY\tTargetZ\tActualX\tActualY\tActualZ\tDistance\n";
    }
    Body *body;
    Geom *geom;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    Marker *marker;
    const double *r = nullptr;
    double err = 0;
    dVector3 v;

    int valueListIndex = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), simulation()->GetTime());
    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetPosition();
        err = (pgd::Vector(r[0], r[1], r[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldPosition(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
        r = v;
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
        r = v;
    }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
        r = v;
    }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
        r = v;
    }
    else if ((marker = dynamic_cast<Marker *>(GetTarget())) != nullptr)
    {
        pgd::Vector vec = marker->GetWorldPosition();
        v[0] = vec.x; v[1] = vec.y; v[2] = vec.z;
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[size_t(valueListIndex)]).Magnitude();
        r = v;
    }

    ss << simulation()->GetTime() <<
          "\t" << m_VValueList[size_t(valueListIndex)].x << "\t" << m_VValueList[size_t(valueListIndex)].y << "\t" << m_VValueList[size_t(valueListIndex)].z <<
          "\t" << r[0] << "\t" << r[1] << "\t" << r[2] <<
          "\t" << err <<
          "\n";
    return ss.str();
}

void DataTargetVector::SetTarget(NamedObject *target)
{
    m_Target = target;
}

NamedObject *DataTargetVector::GetTarget()
{
    return m_Target;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *DataTargetVector::createFromAttributes()
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
        auto iterJoint = simulation()->GetJointList()->find(buf);
        if (iterJoint != simulation()->GetJointList()->end()) { m_Target = iterJoint->second.get(); break; }
        auto iterGeom = simulation()->GetGeomList()->find(buf);
        if (iterGeom != simulation()->GetGeomList()->end()) { m_Target = iterGeom->second.get(); break; }
        auto iterMarker = simulation()->GetMarkerList()->find(buf);
        if (iterMarker != simulation()->GetMarkerList()->end()) { m_Target = iterMarker->second.get(); break; }
    }
    if (!m_Target)
    {
        setLastError("DataTargetVector ID=\""s + name() +"\" TargetID not found "s + buf);
        return lastErrorPtr();
    }

    if (findAttribute("TargetValues"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> targetValuesTokens;
    pystring::split(buf, targetValuesTokens);
    if (targetValuesTokens.size() == 0)
    {
        setLastError("DataTargetVector ID=\""s + name() +"\" No values found in TargetValues"s);
        return lastErrorPtr();
    }
    if (int(targetValuesTokens.size()) != TargetTimeListLength() * 3)
    {
        setLastError("DataTargetVector ID=\""s + name() +"\" Number of values in TargetValues does not match 3 * TargetTimes"s);
        return lastErrorPtr();
    }
    std::vector<double> targetValues;
    targetValues.reserve(targetValuesTokens.size());
    for (auto token : targetValuesTokens) targetValues.push_back(GSUtil::Double(token));
    SetTargetValues(int(targetValues.size()) / 3, targetValues.data());

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void DataTargetVector::appendToAttributes()
{
    DataTarget::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Scalar"s);
    std::vector<double> valueList;
    valueList.reserve(size_t(m_VValueListLength) * 3);
    for (size_t i = 0; i < size_t(m_VValueListLength); i++)
    {
        valueList.push_back(m_VValueList[i].x);
        valueList.push_back(m_VValueList[i].y);
        valueList.push_back(m_VValueList[i].z);
    }
    setAttribute("TargetValues"s, *GSUtil::ToString(valueList.data(), valueList.size(), &buf));
    setAttribute("TargetID"s, m_Target->name());
}

