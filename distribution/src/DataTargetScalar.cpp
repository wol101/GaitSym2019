/*
 *  DataTargetScalar.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
 */

#include <iostream>

#include "ode/ode.h"

#include "DataTargetScalar.h"
#include "Body.h"
#include "HingeJoint.h"
#include "BallJoint.h"
#include "UniversalJoint.h"
#include "PositionReporter.h"
#include "Geom.h"
#include "GSUtil.h"
#include "TegotaeDriver.h"

DataTargetScalar::DataTargetScalar()
{
    m_DataType = XP;
}

DataTargetScalar::~DataTargetScalar()
{
}

// returns the difference between the target actual value and the desired value (actual - desired)
double DataTargetScalar::GetError(int index)
{
    double errorScore = 0;
    const double *r;
    dVector3 result;
    dQuaternion q;
    pgd::Quaternion pq;
    pgd::Vector pv;

    Body *body;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    PositionReporter *positionReporter;
    Geom *geom;
    TegotaeDriver *tegotaeDriver;

    if (index < 0) index = 0;
    if (index >= ValueListLength())
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
            errorScore = (r[0] - ValueList()[index]);
            break;
        case Q1:
            r = body->GetQuaternion();
            errorScore = (r[1] - ValueList()[index]);
            break;
        case Q2:
            r = body->GetQuaternion();
            errorScore = (r[2] - ValueList()[index]);
            break;
        case Q3:
            r = body->GetQuaternion();
            errorScore = (r[3] - ValueList()[index]);
            break;
        case XP:
            r = body->GetPosition();
            errorScore = (r[0] - ValueList()[index]);
            break;
        case YP:
            r = body->GetPosition();
            errorScore = (r[1] - ValueList()[index]);
            break;
        case ZP:
            r = body->GetPosition();
            errorScore = (r[2] - ValueList()[index]);
            break;
        case XV:
            r = body->GetLinearVelocity();
            errorScore = (r[0] - ValueList()[index]);
            break;
        case YV:
            r = body->GetLinearVelocity();
            errorScore = (r[1] - ValueList()[index]);
            break;
        case ZV:
            r = body->GetLinearVelocity();
            errorScore = (r[2] - ValueList()[index]);
            break;
        case XRV:
            r = body->GetAngularVelocity();
            errorScore = (r[0] - ValueList()[index]);
            break;
        case YRV:
            r = body->GetAngularVelocity();
            errorScore = (r[1] - ValueList()[index]);
            break;
        case ZRV:
            r = body->GetAngularVelocity();
            errorScore = (r[2] - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((positionReporter = dynamic_cast<PositionReporter *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.n - ValueList()[index]);
            break;
        case Q1:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.v.x - ValueList()[index]);
            break;
        case Q2:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.v.y - ValueList()[index]);
            break;
        case Q3:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.v.z - ValueList()[index]);
            break;
        case XP:
            pv = positionReporter->GetWorldPosition();
            errorScore = (pv.x - ValueList()[index]);
            break;
        case YP:
            pv = positionReporter->GetWorldPosition();
            errorScore = (pv.y - ValueList()[index]);
            break;
        case ZP:
            pv = positionReporter->GetWorldPosition();
            errorScore = (pv.z - ValueList()[index]);
            break;
        case XV:
            pv = positionReporter->GetWorldVelocity();
            errorScore = (pv.x - ValueList()[index]);
            break;
        case YV:
            pv = positionReporter->GetWorldVelocity();
            errorScore = (pv.y - ValueList()[index]);
            break;
        case ZV:
            pv = positionReporter->GetWorldVelocity();
            errorScore = (pv.z - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(result);
        switch (m_DataType)
        {
        case XP:
            errorScore = (result[0] - ValueList()[index]);
            break;
        case YP:
            errorScore = (result[1] - ValueList()[index]);
            break;
        case ZP:
            errorScore = (result[2] - ValueList()[index]);
            break;
        case Angle:
            errorScore = (hingeJoint->GetHingeAngle() - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(result);
        switch (m_DataType)
        {
        case XP:
            errorScore = (result[0] - ValueList()[index]);
            break;
        case YP:
            errorScore = (result[1] - ValueList()[index]);
            break;
        case ZP:
            errorScore = (result[2] - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(result);
        switch (m_DataType)
        {
        case XP:
            errorScore = (result[0] - ValueList()[index]);
            break;
        case YP:
            errorScore = (result[1] - ValueList()[index]);
            break;
        case ZP:
            errorScore = (result[2] - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            geom->GetWorldQuaternion(q);
            errorScore = (q[0] - ValueList()[index]);
            break;
        case Q1:
            geom->GetWorldQuaternion(q);
            errorScore = (q[1] - ValueList()[index]);
            break;
        case Q2:
            geom->GetWorldQuaternion(q);
            errorScore = (q[2] - ValueList()[index]);
            break;
        case Q3:
            geom->GetWorldQuaternion(q);
            errorScore = (q[3] - ValueList()[index]);
            break;
        case XP:
            geom->GetWorldPosition(result);
            errorScore = (result[0] - ValueList()[index]);
            break;
        case YP:
            geom->GetWorldPosition(result);
            errorScore = (result[1] - ValueList()[index]);
            break;
        case ZP:
            geom->GetWorldPosition(result);
            errorScore = (result[2] - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            geom->GetWorldQuaternion(q);
            errorScore = (q[0] - ValueList()[index]);
            break;
        case Q1:
            geom->GetWorldQuaternion(q);
            errorScore = (q[1] - ValueList()[index]);
            break;
        case Q2:
            geom->GetWorldQuaternion(q);
            errorScore = (q[2] - ValueList()[index]);
            break;
        case Q3:
            geom->GetWorldQuaternion(q);
            errorScore = (q[3] - ValueList()[index]);
            break;
        case XP:
            geom->GetWorldPosition(result);
            errorScore = (result[0] - ValueList()[index]);
            break;
        case YP:
            geom->GetWorldPosition(result);
            errorScore = (result[1] - ValueList()[index]);
            break;
        case ZP:
            geom->GetWorldPosition(result);
            errorScore = (result[2] - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((tegotaeDriver = dynamic_cast<TegotaeDriver *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case DriverError:
            double X = tegotaeDriver->X();
            double Y = tegotaeDriver->Y();
            errorScore = sqrt(X*X + Y*Y) - ValueList()[index];
            break;
        }
    }
    else if (GetTarget() == nullptr)
    {
        switch(m_DataType)
        {
        case MetabolicEnergy:
            errorScore = (simulation()->GetMetabolicEnergy() - ValueList()[index]);
            break;
        case MechanicalEnergy:
            errorScore = (simulation()->GetMechanicalEnergy() - ValueList()[index]);
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else
    {
        std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataTarget " << m_DataType << "\n";
    }

    return errorScore;
}
// returns the difference between the target actual value and the desired value (actual - desired)
double DataTargetScalar::GetError(double time)
{
    double errorScore = 0;
    const double *r;
    dVector3 result;
    dQuaternion q;
    pgd::Quaternion pq;
    pgd::Vector pv;

    Body *body;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    PositionReporter *positionReporter;
    Geom *geom;
    TegotaeDriver *tegotaeDriver;

    int index = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), time);
    if (index < 0) index = 0;
    if (index >= ValueListLength() - 1)
    {
        std::cerr << "Warning: DataTargetScalar::GetMatchValue index out of range\n";
        return 0;
    }
    int indexNext = index + 1;

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            r = body->GetQuaternion();
            errorScore = (r[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q1:
            r = body->GetQuaternion();
            errorScore = (r[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q2:
            r = body->GetQuaternion();
            errorScore = (r[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q3:
            r = body->GetQuaternion();
            errorScore = (r[3] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case XP:
            r = body->GetPosition();
            errorScore = (r[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YP:
            r = body->GetPosition();
            errorScore = (r[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZP:
            r = body->GetPosition();
            errorScore = (r[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case XV:
            r = body->GetLinearVelocity();
            errorScore = (r[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YV:
            r = body->GetLinearVelocity();
            errorScore = (r[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZV:
            r = body->GetLinearVelocity();
            errorScore = (r[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case XRV:
            r = body->GetAngularVelocity();
            errorScore = (r[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YRV:
            r = body->GetAngularVelocity();
            errorScore = (r[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZRV:
            r = body->GetAngularVelocity();
            errorScore = (r[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((positionReporter = dynamic_cast<PositionReporter *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.n - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q1:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.v.x - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q2:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.v.y - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q3:
            pq = positionReporter->GetWorldQuaternion();
            errorScore = (pq.v.z - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case XP:
            pv = positionReporter->GetWorldPosition();
            errorScore = (pv.x - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YP:
            pv = positionReporter->GetWorldPosition();
            errorScore = (pv.y - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZP:
            pv = positionReporter->GetWorldPosition();
            errorScore = (pv.z - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case XV:
            pv = positionReporter->GetWorldVelocity();
            errorScore = (pv.x - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YV:
            pv = positionReporter->GetWorldVelocity();
            errorScore = (pv.y - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZV:
            pv = positionReporter->GetWorldVelocity();
            errorScore = (pv.z - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(result);
        switch (m_DataType)
        {
        case XP:
            errorScore = (result[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YP:
            errorScore = (result[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZP:
            errorScore = (result[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Angle:
            errorScore = (hingeJoint->GetHingeAngle() - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(result);
        switch (m_DataType)
        {
        case XP:
            errorScore = (result[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YP:
            errorScore = (result[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZP:
            errorScore = (result[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(result);
        switch (m_DataType)
        {
        case XP:
            errorScore = (result[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YP:
            errorScore = (result[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZP:
            errorScore = (result[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case Q0:
            geom->GetWorldQuaternion(q);
            errorScore = (q[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q1:
            geom->GetWorldQuaternion(q);
            errorScore = (q[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q2:
            geom->GetWorldQuaternion(q);
            errorScore = (q[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case Q3:
            geom->GetWorldQuaternion(q);
            errorScore = (q[3] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case XP:
            geom->GetWorldPosition(result);
            errorScore = (result[0] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case YP:
            geom->GetWorldPosition(result);
            errorScore = (result[1] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case ZP:
            geom->GetWorldPosition(result);
            errorScore = (result[2] - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else if ((tegotaeDriver = dynamic_cast<TegotaeDriver *>(GetTarget())) != nullptr)
    {
        switch (m_DataType)
        {
        case DriverError:
            errorScore = tegotaeDriver->GetValue(time) - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time);
        }
    }
    else if (GetTarget() == 0)
    {
        switch(m_DataType)
        {
        case MetabolicEnergy:
            errorScore = (simulation()->GetMetabolicEnergy() - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        case MechanicalEnergy:
            errorScore = (simulation()->GetMechanicalEnergy() - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[indexNext], ValueList()[indexNext], time));
            break;
        default:
            std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataType " << m_DataType << "\n";
        }
    }
    else
    {
        std::cerr << "DataTargetScalar::GetMatchValue error in " << GetName() << " unknown DataTarget " << m_DataType << "\n";
    }

    return errorScore;
}


void DataTargetScalar::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == 0)
        {
            if (GetName().size() == 0) std::cerr << "NamedObject::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tTargetV\tActualV\tError\n";
        }
    }

    Body *body;
    Geom *geom;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    const double *r;
    double ref = 0;
    dVector3 result;
    dQuaternion q;
    TegotaeDriver *tegotaeDriver;

    if (dumpStream())
    {
        if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
        {
            switch (m_DataType)
            {
            case Q0:
                r = body->GetQuaternion();
                ref = r[0];
                break;
            case Q1:
                r = body->GetQuaternion();
                ref = r[1];
                break;
            case Q2:
                r = body->GetQuaternion();
                ref = r[2];
                break;
            case Q3:
                r = body->GetQuaternion();
                ref = r[3];
                break;
            case XP:
                r = body->GetPosition();
                ref = r[0];
                break;
            case YP:
                r = body->GetPosition();
                ref = r[1];
                break;
            case ZP:
                r = body->GetPosition();
                ref = r[2];
                break;
            case XV:
                r = body->GetLinearVelocity();
                ref = r[0];
                break;
            case YV:
                r = body->GetLinearVelocity();
                ref = r[1];
                break;
            case ZV:
                r = body->GetLinearVelocity();
                ref = r[2];
                break;
            case XRV:
                r = body->GetAngularVelocity();
                ref = r[0];
                break;
            case YRV:
                r = body->GetAngularVelocity();
                ref = r[1];
                break;
            case ZRV:
                r = body->GetAngularVelocity();
                ref = r[2];
                break;
            default:
                break;
            }
        }
        else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
        {
            hingeJoint->GetHingeAnchor(result);
            switch (m_DataType)
            {
            case XP:
                ref = result[0];
                break;
            case YP:
                ref = result[1];
                break;
            case ZP:
                ref = result[2];
                break;
            case Angle:
                ref = hingeJoint->GetHingeAngle();
            break;
            default:
                break;
            }
        }
        else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
        {
            ballJoint->GetBallAnchor(result);
            switch (m_DataType)
            {
            case XP:
                ref = result[0];
                break;
            case YP:
                ref = result[1];
                break;
            case ZP:
                ref = result[2];
                break;
            default:
                break;
            }
        }
        else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
        {
            switch (m_DataType)
            {
            case Q0:
                geom->GetWorldQuaternion(q);
                ref = q[0];
                break;
            case Q1:
                geom->GetWorldQuaternion(q);
                ref = q[1];
                break;
            case Q2:
                geom->GetWorldQuaternion(q);
                ref = q[2];
                break;
            case Q3:
                geom->GetWorldQuaternion(q);
                ref = q[3];
                break;
            case XP:
                geom->GetWorldPosition(result);
                ref = result[0];
                break;
            case YP:
                geom->GetWorldPosition(result);
                ref = result[1];
                break;
            case ZP:
                geom->GetWorldPosition(result);
                ref = result[2];
                break;
            default:
                break;
            }
        }
        else if ((tegotaeDriver = dynamic_cast<TegotaeDriver *>(GetTarget())) != nullptr)
        {
            switch (m_DataType)
            {
            case DriverError:
                double X = tegotaeDriver->X();
                double Y = tegotaeDriver->Y();
                ref = sqrt(X*X + Y*Y);
            }
        }
        else if (GetTarget() == 0)
        {
            switch(m_DataType)
            {
            case MetabolicEnergy:
                ref = simulation()->GetMetabolicEnergy();
                break;
            case MechanicalEnergy:
                ref = simulation()->GetMechanicalEnergy();
                break;
            default:
                break;
            }
        }

        int index = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), simulation()->GetTime());
        *dumpStream() << simulation()->GetTime() <<
                "\t" << GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[index + 1], ValueList()[index + 1], simulation()->GetTime()) <<
                "\t" << ref <<
                "\t" << ref - GSUtil::Interpolate(TargetTimeList()[index], ValueList()[index], TargetTimeList()[index + 1], ValueList()[index + 1], simulation()->GetTime()) <<
                "\n";
    }
}

