/*
 *  Strap.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "Strap.h"
#include "Body.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "Marker.h"

#include "pystring.h"

#include <string>
#include <sstream>

using namespace std::string_literals;

Strap::Strap()
{
}

Strap::~Strap()
{
}

std::string *Strap::createFromAttributes()
{
    m_pointForceList.clear();
    m_torqueMarkerList.clear();
    if (NamedObject::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (findAttribute("Length"s, &buf) == nullptr) return lastErrorPtr();
    this->setLength(GSUtil::Double(buf));

    if (findAttribute("TorqueMarkerIDList"s, &buf))
    {
        std::vector<std::string> result;
        pystring::split(buf, result);
        if (result.size())
        {
            m_torqueMarkerList.reserve(result.size());
            for (auto &&it: result)
            {
                auto torqueMarker = simulation()->GetMarkerList()->find(it);
                if (torqueMarker == simulation()->GetMarkerList()->end())
                {
                    setLastError("STRAP ID=\""s + name() +"\" torque marker \""s + it +"\" not found"s);
                    return lastErrorPtr();
                }
                m_torqueMarkerList.push_back(torqueMarker->second.get());
            }
        }
    }
    return nullptr;
}

void Strap::saveToAttributes()
{
    this->setTag("STRAP"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

void Strap::appendToAttributes()
{
    NamedObject::appendToAttributes();
    std::string buf;
    setAttribute("Length"s, *GSUtil::ToString(m_length, &buf));
    std::vector<std::string> markerNames;
    markerNames.reserve(m_torqueMarkerList.size());
    for (auto &&it: m_torqueMarkerList) markerNames.push_back(it->name());
    std::string torqueMarkerList = pystring::join(" "s, markerNames);
    setAttribute("TorqueMarkerIDList"s, torqueMarkerList);
    return;
}

double Strap::Length() const
{
    return m_length;
}

double Strap::Velocity() const
{
    return m_velocity;
}

double Strap::Tension() const
{
    return m_tension;
}

void Strap::setTension(double Tension)
{
    m_tension = Tension;
}

std::vector<Marker *> Strap::torqueMarkerList() const
{
    return m_torqueMarkerList;
}

void Strap::setTorqueMarkerList(const std::vector<Marker *> &torqueMarkerList)
{
    m_torqueMarkerList = torqueMarkerList;
}

void Strap::setLength(double Length)
{
    m_length = Length;
}

void Strap::setVelocity(double Velocity)
{
    m_velocity = Velocity;
}

double Strap::GetLength() const
{
    return m_length;
}

double Strap::GetVelocity() const
{
    return m_velocity;
}

void Strap::SetTension(double tension)
{
    m_tension = tension;
}

double Strap::GetTension() const
{
    return m_tension;
}

std::vector<std::unique_ptr<PointForce> > *Strap::GetPointForceList()
{
    return &m_pointForceList;
}

void Strap::GetTorque(const Marker &marker, pgd::Vector3 *worldTorque, pgd::Vector3 *markerTorque, pgd::Vector3 *worldMomentArm, pgd::Vector3 *markerMomentArm)
{
    // Theory:
    // Point = point of force effect
    // Center = center of gravity/rotation
    // Direction = direction of force effect (unit)
    // Force_Scalar = amount of force effect
    // Torque = cross(Point - Center, Direction * Force_Scalar)
    // Axis = normalize(cross(Point - Center, Direction));
    // Torque_Scalar = dot(Point - Center, Cross(Direction, Axis)) * Force_Scalar;

    // sum the torques acting on body 0 of the joint
    pgd::Vector3 torque, point, force, centre;
    double *forcePoint, *forceDirection;
    pgd::Vector3 totalTorque, momentArm;
//    dVector3 result;
//    dBodyGetRelPointPos(mBody->GetBodyID(), mPivotPoint.x, mPivotPoint.y, mPivotPoint.z, result); // needs to be in world coordinates
//    centre = pgd::Vector3(result[0], result[1], result[2]);
    centre = marker.GetWorldPosition();

// These are the same but the second option works even when tension is zero
//        if (m_Tension > 0)
//        {
//            for (unsigned int i = 0; i < m_PointForceList.size(); i++)
//            {
//                if (m_PointForceList[i]->body == marker.GetBody())
//                {
//                    //Torque = cross(Point - Center, Force)
//                    forcePoint = m_PointForceList[i]->point;
//                    point = pgd::Vector3(forcePoint[0], forcePoint[1], forcePoint[2]);
//                    forceDirection = m_PointForceList[i]->vector;
//                    force = pgd::Vector3(forceDirection[0], forceDirection[1], forceDirection[2]) * m_Tension;
//                    torque = (point - centre) ^ force;
//                    totalTorque += torque;
//                }
//            }
//            momentArm = totalTorque / m_Tension;
//        }
//        else
      {
        for (unsigned int i = 0; i < m_pointForceList.size(); i++)
        {
            if (m_pointForceList[i]->body == marker.GetBody())
            {
                //Torque = cross(Point - Center, Force)
                forcePoint = m_pointForceList[i]->point;
                point = pgd::Vector3(forcePoint[0], forcePoint[1], forcePoint[2]);
                forceDirection = m_pointForceList[i]->vector;
                force = pgd::Vector3(forceDirection[0], forceDirection[1], forceDirection[2]);
                torque = (point - centre) ^ force;
                momentArm += torque;
            }
        }
        totalTorque = momentArm * m_tension;
    }

//    // convert to body local coordinates
//    dVector3 bodyTorque, bodyMomentArm;
//    dBodyVectorFromWorld (mBody->GetBodyID(), totalTorque.x, totalTorque.y, totalTorque.z, bodyTorque);
//    dBodyVectorFromWorld (mBody->GetBodyID(), momentArm.x, momentArm.y, momentArm.z, bodyMomentArm);

//    // now find the rotation axis specific values
//    pgd::Matrix3x3 R;
//    CalculateRotationFromAxis(mAxis.x, mAxis.y, mAxis.z, &R);
//    pgd::Vector3 axisBasedTorque = R * totalTorque;
//    pgd::Vector3 axisBasedMomentArm = R * momentArm;

    *worldTorque = totalTorque;
    *worldMomentArm = momentArm;
    pgd::Quaternion q = marker.GetWorldQuaternion();
    *markerTorque = pgd::QVRotate(q, *worldTorque);
    *markerMomentArm = pgd::QVRotate(q, *worldMomentArm);
}

std::string Strap::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
         ss << "Time\tLength";
#pragma warning( suppress : 4189 ) // suppresses 'warning C4189: 'it': local variable is initialized but not referenced' for one line
        for (auto &&it : m_pointForceList) ss << "\tBody\tXP\tYP\tZP\tFX\tFY\tFZ";
#pragma warning( suppress : 4189 ) // suppresses 'warning C4189: 'it': local variable is initialized but not referenced' for one line
        for (auto &&it : m_torqueMarkerList) ss << "\tMarker\tWTX\tWTY\tWTZ\tMTX\tMTY\tMTZ\tWMAX\tWMAY\tWMAZ\tMMAX\tMMAY\tMMAZ";
        ss << "\n";
    }
    ss << simulation()->GetTime() << "\t" << GetLength();
    for (auto &&it: m_pointForceList)
    {
        ss << "\t" << it->body->name() << "\t" <<
              it->point[0] << "\t" << it->point[1] << "\t" << it->point[2] << "\t" <<
              it->vector[0] * m_tension << "\t" << it->vector[1] * m_tension << "\t" << it->vector[2] * m_tension;
    }
    pgd::Vector3 worldTorque,markerTorque, worldMomentArm, markerMomentArm;
    for (auto &&it: m_torqueMarkerList)
    {
        GetTorque(*it, &worldTorque, &markerTorque, &worldMomentArm, &markerMomentArm);
        ss << "\t" << it->name() << "\t" <<
              worldTorque.x << "\t" << worldTorque.y << "\t" << worldTorque.z << "\t" <<
              markerTorque.x << "\t" << markerTorque.y << "\t" << markerTorque.z << "\t" <<
              worldMomentArm.x << "\t" << worldMomentArm.y << "\t" << worldMomentArm.z << "\t" <<
              markerMomentArm.x << "\t" << markerMomentArm.y << "\t" << markerMomentArm.z;
    }
    ss << "\n";
    return ss.str();
}





