/**
 *  @file DataTargetScalar.cpp
 *  @author Bill Sellers
 *  @date Tue July 14 2009.
 *  @copyright Copyright (c) 2009 Bill Sellers. All rights reserved.
 *  @brief Data target that produces a score based on a scalar value.
 *
 *  See the createFromAttributes description for details of the XML specification.
 *
 *  See the description of dumpToString for details of the output file format for this element.
 */

#include "DataTargetScalar.h"
#include "Body.h"
#include "HingeJoint.h"
#include "BallJoint.h"
#include "UniversalJoint.h"
#include "Marker.h"
#include "Geom.h"
#include "GSUtil.h"
#include "TegotaeDriver.h"

#include "pystring.h"

#include <sstream>
#include <algorithm>

using namespace std::string_literals;

DataTargetScalar::DataTargetScalar()
{
}

DataTargetScalar::~DataTargetScalar()
{
}

// returns the difference between the target actual value and the desired value (actual - desired)
double DataTargetScalar::calculateError(size_t index)
{
    if (index >= m_ValueList.size())
    {
        std::cerr << "Warning: DataTargetScalar::GetMatchValue index out of range\n";
        return 0;
    }

    double value = m_ValueList[size_t(index)];
    return calculateErrorScore(value);
}


double DataTargetScalar::calculateErrorScore(double value)
{
    m_errorScore = 0;
    const double *r;
    dVector3 result;
    dQuaternion q;
    pgd::Quaternion pq;
    pgd::Vector3 pv;
    std::vector<Contact *> *contactList;
    double force;

    Body *body;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    Marker *marker;
    Geom *geom;
    TegotaeDriver *tegotaeDriver;

    while (true)
    {
        if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
        {
            switch (m_DataType)
            {
            case Q0:
                r = body->GetQuaternion();
                m_errorScore = (r[0] - value);
                break;
            case Q1:
                r = body->GetQuaternion();
                m_errorScore = (r[1] - value);
                break;
            case Q2:
                r = body->GetQuaternion();
                m_errorScore = (r[2] - value);
                break;
            case Q3:
                r = body->GetQuaternion();
                m_errorScore = (r[3] - value);
                break;
            case XP:
                r = body->GetPosition();
                m_errorScore = (r[0] - value);
                break;
            case YP:
                r = body->GetPosition();
                m_errorScore = (r[1] - value);
                break;
            case ZP:
                r = body->GetPosition();
                m_errorScore = (r[2] - value);
                break;
            case XV:
                r = body->GetLinearVelocity();
                m_errorScore = (r[0] - value);
                break;
            case YV:
                r = body->GetLinearVelocity();
                m_errorScore = (r[1] - value);
                break;
            case ZV:
                r = body->GetLinearVelocity();
                m_errorScore = (r[2] - value);
                break;
            case XRV:
                r = body->GetAngularVelocity();
                m_errorScore = (r[0] - value);
                break;
            case YRV:
                r = body->GetAngularVelocity();
                m_errorScore = (r[1] - value);
                break;
            case ZRV:
                r = body->GetAngularVelocity();
                m_errorScore = (r[2] - value);
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        if ((marker = dynamic_cast<Marker *>(GetTarget())) != nullptr)
        {
            switch (m_DataType)
            {
            case Q0:
                pq = marker->GetWorldQuaternion();
                m_errorScore = (pq.n - value);
                break;
            case Q1:
                pq = marker->GetWorldQuaternion();
                m_errorScore = (pq.x - value);
                break;
            case Q2:
                pq = marker->GetWorldQuaternion();
                m_errorScore = (pq.y - value);
                break;
            case Q3:
                pq = marker->GetWorldQuaternion();
                m_errorScore = (pq.z - value);
                break;
            case XP:
                pv = marker->GetWorldPosition();
                m_errorScore = (pv.x - value);
                break;
            case YP:
                pv = marker->GetWorldPosition();
                m_errorScore = (pv.y - value);
                break;
            case ZP:
                pv = marker->GetWorldPosition();
                m_errorScore = (pv.z - value);
                break;
            case XV:
                pv = marker->GetWorldLinearVelocity();
                m_errorScore = (pv.x - value);
                break;
            case YV:
                pv = marker->GetWorldLinearVelocity();
                m_errorScore = (pv.y - value);
                break;
            case ZV:
                pv = marker->GetWorldLinearVelocity();
                m_errorScore = (pv.z - value);
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
        {
            hingeJoint->GetHingeAnchor(result);
            switch (m_DataType)
            {
            case XP:
                m_errorScore = (result[0] - value);
                break;
            case YP:
                m_errorScore = (result[1] - value);
                break;
            case ZP:
                m_errorScore = (result[2] - value);
                break;
            case Angle:
                m_errorScore = (hingeJoint->GetHingeAngle() - value);
                break;
            case XF:
                force = hingeJoint->GetFeedback()->f1[0];
                m_errorScore = (force - value);
                break;
            case YF:
                force = hingeJoint->GetFeedback()->f1[2];
                m_errorScore = (force - value);
                break;
            case ZF:
                force = hingeJoint->GetFeedback()->f1[3];
                m_errorScore = (force - value);
                break;
            case Force:
                pv.Set(hingeJoint->GetFeedback()->f1);
                force = pv.Magnitude();
                m_errorScore = (force - value);
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
        {
            ballJoint->GetBallAnchor(result);
            switch (m_DataType)
            {
            case XP:
                m_errorScore = (result[0] - value);
                break;
            case YP:
                m_errorScore = (result[1] - value);
                break;
            case ZP:
                m_errorScore = (result[2] - value);
                break;
            case XF:
                force = ballJoint->GetFeedback()->f1[0];
                m_errorScore = (force - value);
                break;
            case YF:
                force = ballJoint->GetFeedback()->f1[2];
                m_errorScore = (force - value);
                break;
            case ZF:
                force = ballJoint->GetFeedback()->f1[3];
                m_errorScore = (force - value);
                break;
            case Force:
                pv.Set(ballJoint->GetFeedback()->f1);
                force = pv.Magnitude();
                m_errorScore = (force - value);
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
        {
            universalJoint->GetUniversalAnchor(result);
            switch (m_DataType)
            {
            case XP:
                m_errorScore = (result[0] - value);
                break;
            case YP:
                m_errorScore = (result[1] - value);
                break;
            case ZP:
                m_errorScore = (result[2] - value);
                break;
            case XF:
                force = universalJoint->GetFeedback()->f1[0];
                m_errorScore = (force - value);
                break;
            case YF:
                force = universalJoint->GetFeedback()->f1[2];
                m_errorScore = (force - value);
                break;
            case ZF:
                force = universalJoint->GetFeedback()->f1[3];
                m_errorScore = (force - value);
                break;
            case Force:
                pv.Set(universalJoint->GetFeedback()->f1);
                force = pv.Magnitude();
                m_errorScore = (force - value);
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
        {
            switch (m_DataType)
            {
            case Q0:
                geom->GetWorldQuaternion(q);
                m_errorScore = (q[0] - value);
                break;
            case Q1:
                geom->GetWorldQuaternion(q);
                m_errorScore = (q[1] - value);
                break;
            case Q2:
                geom->GetWorldQuaternion(q);
                m_errorScore = (q[2] - value);
                break;
            case Q3:
                geom->GetWorldQuaternion(q);
                m_errorScore = (q[3] - value);
                break;
            case XP:
                geom->GetWorldPosition(result);
                m_errorScore = (result[0] - value);
                break;
            case YP:
                geom->GetWorldPosition(result);
                m_errorScore = (result[1] - value);
                break;
            case ZP:
                geom->GetWorldPosition(result);
                m_errorScore = (result[2] - value);
                break;
            case XF:
                contactList = geom->GetContactList();
                force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[0];
                m_errorScore = (force - value);
                break;
            case YF:
                contactList = geom->GetContactList();
                force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[1];
                m_errorScore = (force - value);
                break;
            case ZF:
                contactList = geom->GetContactList();
                force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[2];
                m_errorScore = (force - value);
                break;
            case Force:
                contactList = geom->GetContactList();
                force = 0;
                for (auto &&it : *contactList) { pv.Set(it->GetJointFeedback()->f1); force += pv.Magnitude(); }
                m_errorScore = (force - value);
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        if ((tegotaeDriver = dynamic_cast<TegotaeDriver *>(GetTarget())) != nullptr)
        {
            pgd::Vector3 errorVector;
            switch (m_DataType)
            {
            case DriverError:
                errorVector = tegotaeDriver->localErrorVector();
                m_errorScore = errorVector.Magnitude() - value;
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        if (GetTarget() == nullptr)
        {
            switch(m_DataType)
            {
            case MetabolicEnergy:
                m_errorScore = (simulation()->GetMetabolicEnergy() - value);
                break;
            case MechanicalEnergy:
                m_errorScore = (simulation()->GetMechanicalEnergy() - value);
                break;
            case Time:
                m_errorScore = (simulation()->GetTime() - value);
                break;
            case DeltaTime:
                m_errorScore = (simulation()->GetTimeIncrement() - value);
                break;
            default:
                std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
            }
            break;
        }
        std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataTarget " << m_DataType << "\n";
        break;
    }

    switch(std::fpclassify(m_errorScore))
    {
    case FP_NORMAL: break;
    case FP_ZERO: break;
    case FP_INFINITE:
        std::cerr << "Warning: m_errorScore is FP_INFINITE\n";
        m_errorScore = 0;
        break;
    case FP_NAN:
        std::cerr << "Warning: m_errorScore is FP_NAN\n";
        m_errorScore = 0;
        break;
    case FP_SUBNORMAL:
        std::cerr << "Warning: m_errorScore is FP_SUBNORMAL\n";
        m_errorScore = 0;
        break;
    default:
        std::cerr << "Warning: m_errorScore is unclassified\n";
        m_errorScore = 0;
        break;
    }

    return m_errorScore;
}



// returns the difference between the target actual value and the desired value (actual - desired)
double DataTargetScalar::calculateError(double time)
{
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

    double value = GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time);
    return calculateErrorScore(value);
}

/**
 * @brief DataTargetScalar::dumpToString
 * @return string containing the data for this time point
 *
 * This function returns useful data to the user about values contained in this object during the simulation
 *
 * Column Headings:
 *
 * - time
 *   - the simulation time
 * - error
 *   - the raw error generated
 * - score
 *   - the score value generated using the positive function, intersect and slope
 */

std::string DataTargetScalar::dumpToString()
{
    std::string s;
    if (firstDump())
    {
        setFirstDump(false);
        s += dumpHelper({"time"s, "error"s, "score"s});
    }
    s += dumpHelper({simulation()->GetTime(), m_errorScore, lastValue()});
    return s;
}



/**
 * @brief DataTargetScalar::createFromAttributes
 * @return nullptr on success and a pointer to lastError() on failure
 *
 * This function initialises the data in the object based on the contents
 * of an xml_node node. It uses information from the simulation as required
 * to satisfy dependencies
 *
 * Attributes in addition to standard DATATARGET:
 *
 * - Type="Scalar"
 * - DataType="_data type identifier_"
 *   - XP, YP, ZP - position in world coordinates
 *   - Q0, Q1, Q2, Q3 - quaternion in world coodinates
 *   - XV, YV, ZV - linear velocities
 *   - XRV, YRV, ZRV - angular velocities
 *   - Angle - hinge joint angle
 *   - MetabolicEnergy - metabolic energy use
 *   - MechanicalEnergy - mechanical energy
 *   - DriverError - driver error term
 *   - Time - simulation time
 *   - DeltaTime - simulation change of time
 *   - XF, YF, ZF - individual force components
 *   - Force - magnitude of force
 * - TargetID="_ID name_"
 *   - Where _ID name_ is the ID of the target which can be a body, driver, geom, joint, marker or empty depending on the DataType
 *   - Not all data types make sense for all targets and will generate a warning when the simulation is running
 * - TargetValues="_list of numbers_"
 *   - List of numeric values that are used to calculate the error term. The number of values must match the number of TargetTimes.
 */

std::string *DataTargetScalar::createFromAttributes()
{
    if (DataTarget::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    if (findAttribute("DataType"s, &buf) == nullptr) return lastErrorPtr();
    size_t dataTypeIndex;
    for (dataTypeIndex = 0; dataTypeIndex < dataTypeCount; dataTypeIndex++)
    {
        if (buf == dataTypeStrings(dataTypeIndex))
        {
            m_DataType = static_cast<DataType>(dataTypeIndex);
            break;
        }
    }
    if (dataTypeIndex >= dataTypeCount)
    {
        setLastError("DataTargetScalar ID=\""s + name() +"\" Unrecognised MatchType "s + buf);
        return lastErrorPtr();
    }
    m_Target = nullptr;
    if (m_noTargetList.count(m_DataType) == 0)
    {
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
            auto iterDriver = simulation()->GetDriverList()->find(buf);
            if (iterDriver != simulation()->GetDriverList()->end()) { m_Target = iterDriver->second.get(); break; }
        }
        if (!m_Target)
        {
            setLastError("DataTargetScalar ID=\""s + name() +"\" TargetID not found "s + buf);
            return lastErrorPtr();
        }
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

    if (m_Target) setUpstreamObjects({m_Target});
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void DataTargetScalar::appendToAttributes()
{
    DataTarget::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Scalar"s);
    setAttribute("TargetValues"s, *GSUtil::ToString(m_ValueList.data(), m_ValueList.size(), &buf));
    setAttribute("DataType", dataTypeStrings(m_DataType));
    if (m_noTargetList.count(m_DataType) == 0) setAttribute("TargetID"s, m_Target->name());
}

void DataTargetScalar::SetTarget(NamedObject *target)
{
    m_Target = target;
}

NamedObject *DataTargetScalar::GetTarget()
{
    return m_Target;
}



