/*
 *  DataTargetScalar.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
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
    m_errorScore = 0;
    const double *r;
    dVector3 result;
    dQuaternion q;
    pgd::Quaternion pq;
    pgd::Vector3 pv;

    Body *body;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    Marker *marker;
    Geom *geom;
    TegotaeDriver *tegotaeDriver;

    if (index >= m_ValueList.size())
    {
        std::cerr << "Warning: DataTargetScalar::GetMatchValue index out of range\n";
        return 0;
    }

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            r = body->GetQuaternion();
            m_errorScore = (r[0] - m_ValueList[size_t(index)]);
            break;
        case Q1:
            r = body->GetQuaternion();
            m_errorScore = (r[1] - m_ValueList[size_t(index)]);
            break;
        case Q2:
            r = body->GetQuaternion();
            m_errorScore = (r[2] - m_ValueList[size_t(index)]);
            break;
        case Q3:
            r = body->GetQuaternion();
            m_errorScore = (r[3] - m_ValueList[size_t(index)]);
            break;
        case XP:
            r = body->GetPosition();
            m_errorScore = (r[0] - m_ValueList[size_t(index)]);
            break;
        case YP:
            r = body->GetPosition();
            m_errorScore = (r[1] - m_ValueList[size_t(index)]);
            break;
        case ZP:
            r = body->GetPosition();
            m_errorScore = (r[2] - m_ValueList[size_t(index)]);
            break;
        case XV:
            r = body->GetLinearVelocity();
            m_errorScore = (r[0] - m_ValueList[size_t(index)]);
            break;
        case YV:
            r = body->GetLinearVelocity();
            m_errorScore = (r[1] - m_ValueList[size_t(index)]);
            break;
        case ZV:
            r = body->GetLinearVelocity();
            m_errorScore = (r[2] - m_ValueList[size_t(index)]);
            break;
        case XRV:
            r = body->GetAngularVelocity();
            m_errorScore = (r[0] - m_ValueList[size_t(index)]);
            break;
        case YRV:
            r = body->GetAngularVelocity();
            m_errorScore = (r[1] - m_ValueList[size_t(index)]);
            break;
        case ZRV:
            r = body->GetAngularVelocity();
            m_errorScore = (r[2] - m_ValueList[size_t(index)]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((marker = dynamic_cast<Marker *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.n - m_ValueList[size_t(index)]);
            break;
        case Q1:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.x - m_ValueList[size_t(index)]);
            break;
        case Q2:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.y - m_ValueList[size_t(index)]);
            break;
        case Q3:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.z - m_ValueList[size_t(index)]);
            break;
        case XP:
            pv = marker->GetWorldPosition();
            m_errorScore = (pv.x - m_ValueList[size_t(index)]);
            break;
        case YP:
            pv = marker->GetWorldPosition();
            m_errorScore = (pv.y - m_ValueList[size_t(index)]);
            break;
        case ZP:
            pv = marker->GetWorldPosition();
            m_errorScore = (pv.z - m_ValueList[size_t(index)]);
            break;
        case XV:
            pv = marker->GetWorldLinearVelocity();
            m_errorScore = (pv.x - m_ValueList[size_t(index)]);
            break;
        case YV:
            pv = marker->GetWorldLinearVelocity();
            m_errorScore = (pv.y - m_ValueList[size_t(index)]);
            break;
        case ZV:
            pv = marker->GetWorldLinearVelocity();
            m_errorScore = (pv.z - m_ValueList[size_t(index)]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(result);
        switch (m_DataType)
        {
        case XP:
            m_errorScore = (result[0] - m_ValueList[size_t(index)]);
            break;
        case YP:
            m_errorScore = (result[1] - m_ValueList[size_t(index)]);
            break;
        case ZP:
            m_errorScore = (result[2] - m_ValueList[size_t(index)]);
            break;
        case Angle:
            m_errorScore = (hingeJoint->GetHingeAngle() - m_ValueList[size_t(index)]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(result);
        switch (m_DataType)
        {
        case XP:
            m_errorScore = (result[0] - m_ValueList[size_t(index)]);
            break;
        case YP:
            m_errorScore = (result[1] - m_ValueList[size_t(index)]);
            break;
        case ZP:
            m_errorScore = (result[2] - m_ValueList[size_t(index)]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(result);
        switch (m_DataType)
        {
        case XP:
            m_errorScore = (result[0] - m_ValueList[size_t(index)]);
            break;
        case YP:
            m_errorScore = (result[1] - m_ValueList[size_t(index)]);
            break;
        case ZP:
            m_errorScore = (result[2] - m_ValueList[size_t(index)]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[0] - m_ValueList[size_t(index)]);
            break;
        case Q1:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[1] - m_ValueList[size_t(index)]);
            break;
        case Q2:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[2] - m_ValueList[size_t(index)]);
            break;
        case Q3:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[3] - m_ValueList[size_t(index)]);
            break;
        case XP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[0] - m_ValueList[size_t(index)]);
            break;
        case YP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[1] - m_ValueList[size_t(index)]);
            break;
        case ZP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[2] - m_ValueList[size_t(index)]);
            break;
        case XF:
            {
                std::vector<Contact *> *contactList = geom->GetContactList();
                double force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[0];
                m_errorScore = (force - m_ValueList[size_t(index)]);
                break;
            }
        case YF:
            {
                std::vector<Contact *> *contactList = geom->GetContactList();
                double force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[1];
                m_errorScore = (force - m_ValueList[size_t(index)]);
                break;
            }
        case ZF:
            {
                std::vector<Contact *> *contactList = geom->GetContactList();
                double force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[2];
                m_errorScore = (force - m_ValueList[size_t(index)]);
                break;
            }
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[0] - m_ValueList[size_t(index)]);
            break;
        case Q1:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[1] - m_ValueList[size_t(index)]);
            break;
        case Q2:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[2] - m_ValueList[size_t(index)]);
            break;
        case Q3:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[3] - m_ValueList[size_t(index)]);
            break;
        case XP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[0] - m_ValueList[size_t(index)]);
            break;
        case YP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[1] - m_ValueList[size_t(index)]);
            break;
        case ZP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[2] - m_ValueList[size_t(index)]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((tegotaeDriver = dynamic_cast<TegotaeDriver *>(GetTarget())) != nullptr)
    {
        pgd::Vector3 errorVector;
        switch (m_DataType)
        {
        case DriverError:
            errorVector = tegotaeDriver->localErrorVector();
            m_errorScore = errorVector.Magnitude() - m_ValueList[size_t(index)];
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if (GetTarget() == nullptr)
    {
        switch(m_DataType)
        {
        case MetabolicEnergy:
            m_errorScore = (simulation()->GetMetabolicEnergy() - m_ValueList[size_t(index)]);
            break;
        case MechanicalEnergy:
            m_errorScore = (simulation()->GetMechanicalEnergy() - m_ValueList[size_t(index)]);
            break;
        case Time:
            m_errorScore = (simulation()->GetTime() - m_ValueList[size_t(index)]);
            break;
        case DeltaTime:
            m_errorScore = (simulation()->GetTimeIncrement() - m_ValueList[size_t(index)]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else
    {
        std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataTarget " << m_DataType << "\n";
    }

    return m_errorScore;
}

// returns the difference between the target actual value and the desired value (actual - desired)
double DataTargetScalar::calculateError(double time)
{
    m_errorScore = 0;
    const double *r;
    dVector3 result;
    dQuaternion q;
    pgd::Quaternion pq;
    pgd::Vector3 pv;

    Body *body;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    Marker *marker;
    Geom *geom;
    TegotaeDriver *tegotaeDriver;

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

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            r = body->GetQuaternion();
            m_errorScore = (r[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q1:
            r = body->GetQuaternion();
            m_errorScore = (r[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q2:
            r = body->GetQuaternion();
            m_errorScore = (r[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q3:
            r = body->GetQuaternion();
            m_errorScore = (r[3] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case XP:
            r = body->GetPosition();
            m_errorScore = (r[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YP:
            r = body->GetPosition();
            m_errorScore = (r[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZP:
            r = body->GetPosition();
            m_errorScore = (r[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case XV:
            r = body->GetLinearVelocity();
            m_errorScore = (r[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YV:
            r = body->GetLinearVelocity();
            m_errorScore = (r[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZV:
            r = body->GetLinearVelocity();
            m_errorScore = (r[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case XRV:
            r = body->GetAngularVelocity();
            m_errorScore = (r[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YRV:
            r = body->GetAngularVelocity();
            m_errorScore = (r[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZRV:
            r = body->GetAngularVelocity();
            m_errorScore = (r[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((marker = dynamic_cast<Marker *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.n - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q1:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.x - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q2:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.y - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q3:
            pq = marker->GetWorldQuaternion();
            m_errorScore = (pq.z - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case XP:
            pv = marker->GetWorldPosition();
            m_errorScore = (pv.x - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YP:
            pv = marker->GetWorldPosition();
            m_errorScore = (pv.y - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZP:
            pv = marker->GetWorldPosition();
            m_errorScore = (pv.z - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case XV:
            pv = marker->GetWorldLinearVelocity();
            m_errorScore = (pv.x - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YV:
            pv = marker->GetWorldLinearVelocity();
            m_errorScore = (pv.y - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZV:
            pv = marker->GetWorldLinearVelocity();
            m_errorScore = (pv.z - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(result);
        switch (m_DataType)
        {
        case XP:
            m_errorScore = (result[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YP:
            m_errorScore = (result[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZP:
            m_errorScore = (result[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Angle:
            m_errorScore = (hingeJoint->GetHingeAngle() - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(result);
        switch (m_DataType)
        {
        case XP:
            m_errorScore = (result[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YP:
            m_errorScore = (result[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZP:
            m_errorScore = (result[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(result);
        switch (m_DataType)
        {
        case XP:
            m_errorScore = (result[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YP:
            m_errorScore = (result[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZP:
            m_errorScore = (result[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q1:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q2:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Q3:
            geom->GetWorldQuaternion(q);
            m_errorScore = (q[3] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case XP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[0] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case YP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[1] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case ZP:
            geom->GetWorldPosition(result);
            m_errorScore = (result[2] - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case XF:
            {
                std::vector<Contact *> *contactList = geom->GetContactList();
                double force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[0];
                m_errorScore = (force - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
                break;
            }
        case YF:
            {
                std::vector<Contact *> *contactList = geom->GetContactList();
                double force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[1];
                m_errorScore = (force - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
                break;
            }
        case ZF:
            {
                std::vector<Contact *> *contactList = geom->GetContactList();
                double force = 0;
                for (auto &&it : *contactList) force += it->GetJointFeedback()->f1[2];
                m_errorScore = (force - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
                break;
            }
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((tegotaeDriver = dynamic_cast<TegotaeDriver *>(GetTarget())) != nullptr)
    {
        pgd::Vector3 errorVector;
        switch (m_DataType)
        {
        case DriverError:
            errorVector = tegotaeDriver->localErrorVector();
            m_errorScore = errorVector.Magnitude() - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if (GetTarget() == nullptr)
    {
        switch(m_DataType)
        {
        case MetabolicEnergy:
            m_errorScore = (simulation()->GetMetabolicEnergy() - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case MechanicalEnergy:
            m_errorScore = (simulation()->GetMechanicalEnergy() - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case Time:
            m_errorScore = (simulation()->GetTime() - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        case DeltaTime:
            m_errorScore = (simulation()->GetTimeIncrement() - GSUtil::Interpolate((*targetTimeList())[size_t(index)], m_ValueList[size_t(index)], (*targetTimeList())[indexNext], m_ValueList[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else
    {
        std::cerr << "DataTargetScalar::GetMatchValue error in " << name() << " unknown DataTarget " << m_DataType << "\n";
    }

    switch(std::fpclassify(m_errorScore))
    {
    case FP_NORMAL: break;
    case FP_ZERO: break;
    case FP_INFINITE:
        std::cerr << "Warning: m_errorScore is FP_INFINITE\n";
        return 0;
    case FP_NAN:
        std::cerr << "Warning: m_errorScore is FP_NAN\n";
        return 0;
    case FP_SUBNORMAL:
        std::cerr << "Warning: m_errorScore is FP_SUBNORMAL\n";
        return 0;
    default:
        std::cerr << "Warning: m_errorScore is unclassified\n";
        return 0;
    }

    return m_errorScore;
}

std::string DataTargetScalar::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tError\n";
    }
    ss << simulation()->GetTime() << m_errorScore << "\n";
    return ss.str();
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
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



