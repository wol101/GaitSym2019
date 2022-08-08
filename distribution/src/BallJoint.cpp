/*
 *  BallJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/12/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#include "BallJoint.h"
#include "Body.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "Marker.h"

#include "ode/ode.h"

#include <cassert>
#include <cstdlib>
#include <sstream>

using namespace std::string_literals;

BallJoint::BallJoint(dWorldID worldID, Mode mode) : Joint()
{
    // ball joint
    setJointID(dJointCreateBall(worldID, nullptr));
    dJointSetData(JointID(), this);
    dJointSetFeedback(JointID(), JointFeedback());

    // angular motor
    m_MotorJointID = dJointCreateAMotor (worldID, nullptr);
    dJointSetFeedback(m_MotorJointID, &m_MotorJointFeedback);

    m_Mode = mode;
}

void BallJoint::Attach(Body *body1, Body *body2)
{
    assert(body1 != nullptr || body2 != nullptr);
    setBody1(body1);
    setBody2(body2);
    if (GetBody1() == nullptr)
    {
        dJointAttach(JointID(), nullptr, GetBody2()->GetBodyID());
        dJointAttach(m_MotorJointID, nullptr, GetBody2()->GetBodyID());
    }
    else if (GetBody2() == nullptr)
    {
        dJointAttach(JointID(), GetBody1()->GetBodyID(), nullptr);
        dJointAttach(m_MotorJointID, GetBody1()->GetBodyID(), nullptr);
    }
    else
    {
        dJointAttach(JointID(), GetBody1()->GetBodyID(), GetBody2()->GetBodyID());
        dJointAttach(m_MotorJointID, GetBody1()->GetBodyID(), GetBody2()->GetBodyID());
    }
}

void BallJoint::Attach()
{
    this->Attach(body1Marker()->GetBody(), body2Marker()->GetBody());
}

void BallJoint::SetBallAnchor(double x, double y, double z)
{
    dJointSetBallAnchor(JointID(), x, y, z);
}

void BallJoint::GetBallAnchor(dVector3 result)
{
    dJointGetBallAnchor(JointID(), result);
}

void BallJoint::GetBallAnchor2(dVector3 result)
{
    dJointGetBallAnchor2(JointID(), result);
}

// this routine sets up the axes for the joints
// only axis0 and axis2 are used for m_Mode == dAMotorEuler
// axisMode: 0 global, 1 relative to body 1, 2 relative to body 2 (only used in dAMotorUser)
// axes are intially specified globally but move depending on the mode selected
// for dAMotorEuler axis 0 is relative to body 1, and axis 2 is relative to body 2
void BallJoint::SetAxes(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2, int axisMode)
{
    if (m_Mode == static_cast<BallJoint::Mode>(dAMotorEuler))
    {
        dJointSetAMotorMode(m_MotorJointID, dAMotorEuler);
        dJointSetAMotorAxis(m_MotorJointID, 0, 1, x0, y0, z0);
        dJointSetAMotorAxis(m_MotorJointID, 2, 2, x2, y2, z2);
    }
    else if (m_Mode == static_cast<BallJoint::Mode>(dAMotorUser))
    {
        dJointSetAMotorMode(m_MotorJointID, dAMotorUser);
        dJointSetAMotorNumAxes(m_MotorJointID, 3);
        dJointSetAMotorAxis(m_MotorJointID, 0, axisMode, x0, y0, z0);
        dJointSetAMotorAxis(m_MotorJointID, 1, axisMode, x1, y1, z1);
        dJointSetAMotorAxis(m_MotorJointID, 2, axisMode, x2, y2, z2);
    }

//    dVector3 r;

//    if (m_Mode == dAMotorEuler)
//    {
//        dJointSetAMotorMode(m_MotorJointID, dAMotorEuler);
//        dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 0), x0, y0, z0, r);
//        dJointSetAMotorAxis(m_MotorJointID, 0, 1, r[0], r[1], r[2]);
//        dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 1), x2, y2, z2, r);
//        dJointSetAMotorAxis(m_MotorJointID, 2, 2, r[0], r[1], r[2]);
//    }
//    else if (m_Mode == dAMotorUser)
//    {
//        dJointSetAMotorMode(m_MotorJointID, dAMotorUser);
//        dJointSetAMotorNumAxes(m_MotorJointID, 3);
//        if (axisMode == 0)
//        {
//            dJointSetAMotorAxis(m_MotorJointID, 0, axisMode, x0, y0, z0);
//            dJointSetAMotorAxis(m_MotorJointID, 1, axisMode, x1, y1, z1);
//            dJointSetAMotorAxis(m_MotorJointID, 2, axisMode, x2, y2, z2);
//        }
//        else
//        {
//            if (axisMode == 1) // relative to body1
//            {
//                dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 0), x0, y0, z0, r);
//                dJointSetAMotorAxis(m_MotorJointID, 0, 1, r[0], r[1], r[2]);
//                dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 0), x1, y1, z1, r);
//                dJointSetAMotorAxis(m_MotorJointID, 0, 1, r[0], r[1], r[2]);
//                dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 0), x2, y2, z2, r);
//                dJointSetAMotorAxis(m_MotorJointID, 2, 2, r[0], r[1], r[2]);
//            }
//            else // relative to body1
//            {
//                dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 1), x0, y0, z0, r);
//                dJointSetAMotorAxis(m_MotorJointID, 0, 1, r[0], r[1], r[2]);
//                dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 1), x1, y1, z1, r);
//                dJointSetAMotorAxis(m_MotorJointID, 0, 1, r[0], r[1], r[2]);
//                dBodyVectorToWorld(dJointGetBody(m_MotorJointID, 1), x2, y2, z2, r);
//                dJointSetAMotorAxis(m_MotorJointID, 2, 2, r[0], r[1], r[2]);
//            }
//        }
//        // and now can set the angles based on the relative pose of the two bodies
//        SetAngles();
//    }

}

// this routine sets the stops for the joint
// these are relative to the axes specified in SetAxes
void BallJoint::SetStops(double a0Low, double a0High, double a1Low, double a1High, double a2Low, double a2High)
{
    if (m_Mode == static_cast<BallJoint::Mode>(dAMotorEuler) || m_Mode == static_cast<BallJoint::Mode>(dAMotorUser))
    {
        // note there is safety feature that stops setting incompatible low and high
        // stops which can cause difficulties. The safe option is to set them twice.

        dJointSetAMotorParam(m_MotorJointID, dParamLoStop1, a0Low);
        dJointSetAMotorParam(m_MotorJointID, dParamHiStop1, a0High);
        dJointSetAMotorParam(m_MotorJointID, dParamLoStop2, a1Low);
        dJointSetAMotorParam(m_MotorJointID, dParamHiStop2, a1High);
        dJointSetAMotorParam(m_MotorJointID, dParamLoStop3, a2Low);
        dJointSetAMotorParam(m_MotorJointID, dParamHiStop3, a2High);

        dJointSetAMotorParam(m_MotorJointID, dParamLoStop1, a0Low);
        dJointSetAMotorParam(m_MotorJointID, dParamHiStop1, a0High);
        dJointSetAMotorParam(m_MotorJointID, dParamLoStop2, a1Low);
        dJointSetAMotorParam(m_MotorJointID, dParamHiStop2, a1High);
        dJointSetAMotorParam(m_MotorJointID, dParamLoStop3, a2Low);
        dJointSetAMotorParam(m_MotorJointID, dParamHiStop3, a2High);
    }

}

// calculate the angle transformation from body 1 to body 2
//void BallJoint::SetAngles()
//{
//    if (m_Mode == dAMotorUser)
//    {
//        dBodyID body1 = dJointGetBody(m_MotorJointID, 0);
//        dBodyID body2 = dJointGetBody(m_MotorJointID, 1);
//        double thetaX, thetaY, thetaZ;

//        if (body1 == 0) // body 2 is connected to the world so it is already in the correct coodinates
//        {
//            const double *R2 = dBodyGetRotation(body2);
//            GSUtil::EulerDecompositionXYZ(R2, thetaX, thetaY, thetaZ);
//        }
//        else
//        {
//            // find orientation of Body 2 wrt Body 1
//            dMatrix3 rotMat;
//            const double *R1 = dBodyGetRotation(body1);
//            const double *R2 = dBodyGetRotation(body2);
//            dMULTIPLY2_333(rotMat, R2, R1);

//            // now find the X,Y,Z angles (use the Euler formulae for convenience not efficiency)
//            GSUtil::EulerDecompositionXYZ(rotMat, thetaX, thetaY, thetaZ);

//        }

//        dJointSetAMotorAngle(m_MotorJointID, 0, -thetaX);
//        dJointSetAMotorAngle(m_MotorJointID, 1, -thetaY);
//        dJointSetAMotorAngle(m_MotorJointID, 2, -thetaZ);
//    }
//}

// Get the Euler angle reference vectors
// use with care - these values are not generally altered by the user
// and are only used for state save and restore
void BallJoint::GetEulerReferenceVectors(dVector3 reference1, dVector3 reference2)
{
    dJointGetAMotorEulerReferenceVectors( m_MotorJointID , reference1 , reference2 );
}

// Set the Euler angle reference vectors
// use with care - these values are not generally altered by the user
// and are only used for state save and restore
void BallJoint::SetEulerReferenceVectors(dVector3 reference1, dVector3 reference2)
{
    dJointSetAMotorEulerReferenceVectors( m_MotorJointID , reference1 , reference2 );
}

//void BallJoint::Update()
//{
//    if (m_Mode == dAMotorUser)
//    {
//        // this gets called every simulation step so it's a good place to set the angles
//        SetAngles();
//    }
//}

// get the qauternion that rotates from body1 to body2
pgd::Quaternion BallJoint::GetQuaternion()
{
    pgd::Quaternion q;
    body2Marker()->GetBody()->GetRelativeQuaternion(body1Marker()->GetBody(), &q);
    return q;
}


std::string *BallJoint::createFromAttributes()
{
    if (Joint::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (findAttribute("Mode"s, &buf) == nullptr) return lastErrorPtr();
    else if (buf == "NoStops") m_Mode = BallJoint::NoStops;
    else if (buf == "AMotorUser") m_Mode = BallJoint::AMotorUser;
    else if (buf == "AMotorEuler") m_Mode = BallJoint::AMotorEuler;
    else { setLastError("Joint ID=\""s + name() +"\" unrecognised Mode"s); return lastErrorPtr(); }

    pgd::Vector3 position = body1Marker()->GetWorldPosition();
    this->SetBallAnchor(position.x, position.y, position.z);
    pgd::Vector3 x, y, z;
    body1Marker()->GetWorldBasis(&x, &y, &z);
    int axisMode = 1; // 1 relative to body 1
    this->SetAxes(x.x, x.y, x.z, y.x, y.y, y.z, z.x, z.y, z.z, axisMode);
    if (CFM() >= 0) dJointSetBallParam (JointID(), dParamCFM, CFM());
    if (ERP() >= 0) dJointSetBallParam (JointID(), dParamERP, ERP());

    switch (m_Mode)
    {
    case BallJoint::NoStops:
        break;
    case BallJoint::AMotorUser:
        {
            if (findAttribute("LowStop0"s, &buf) == nullptr) return lastErrorPtr();
            double a0Low = GSUtil::GetAngle(buf);
            if (findAttribute("HighStop0"s, &buf) == nullptr) return lastErrorPtr();
            double a0High = GSUtil::GetAngle(buf);
            if (findAttribute("LowStop1"s, &buf) == nullptr) return lastErrorPtr();
            double a1Low = GSUtil::GetAngle(buf);
            if (findAttribute("HighStop1"s, &buf) == nullptr) return lastErrorPtr();
            double a1High = GSUtil::GetAngle(buf);
            if (findAttribute("LowStop2"s, &buf) == nullptr) return lastErrorPtr();
            double a2Low = GSUtil::GetAngle(buf);
            if (findAttribute("HighStop2"s, &buf) == nullptr) return lastErrorPtr();
            double a2High = GSUtil::GetAngle(buf);
            if (a0Low >= a0High) { setLastError("Joint ID=\""s + name() +"\" LowStop0 >= HighStop0"s); return lastErrorPtr(); }
            if (a1Low >= a1High) { setLastError("Joint ID=\""s + name() +"\" LowStop1 >= HighStop1"s); return lastErrorPtr(); }
            if (a2Low >= a2High) { setLastError("Joint ID=\""s + name() +"\" LowStop2 >= HighStop2"s); return lastErrorPtr(); }
            this->SetStops(a0Low, a0High, a1Low, a1High, a2Low, a2High);
            break;
        }
    case BallJoint::AMotorEuler:
        {
            if (findAttribute("LowStop0"s, &buf) == nullptr) return lastErrorPtr();
            double a0Low = GSUtil::GetAngle(buf);
            if (findAttribute("HighStop0"s, &buf) == nullptr) return lastErrorPtr();
            double a0High = GSUtil::GetAngle(buf);
            if (findAttribute("LowStop1"s, &buf) == nullptr) return lastErrorPtr();
            double a1Low = GSUtil::GetAngle(buf);
            if (findAttribute("HighStop1"s, &buf) == nullptr) return lastErrorPtr();
            double a1High = GSUtil::GetAngle(buf);
            if (findAttribute("LowStop2"s, &buf) == nullptr) return lastErrorPtr();
            double a2Low = GSUtil::GetAngle(buf);
            if (findAttribute("HighStop2"s, &buf) == nullptr) return lastErrorPtr();
            double a2High = GSUtil::GetAngle(buf);
            if (a0Low >= a0High) { setLastError("Joint ID=\""s + name() +"\" LowStop0 >= HighStop0"s); return lastErrorPtr(); }
            if (a1Low >= a1High) { setLastError("Joint ID=\""s + name() +"\" LowStop1 >= HighStop1"s); return lastErrorPtr(); }
            if (a2Low >= a2High) { setLastError("Joint ID=\""s + name() +"\" LowStop2 >= HighStop2"s); return lastErrorPtr(); }
            this->SetStops(a0Low, a0High, a1Low, a1High, a2Low, a2High);
            break;
        }
    }
    return nullptr;
}

void BallJoint::appendToAttributes()
{
    Joint::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Ball"s);
    setAttribute("Body1MarkerID"s, body1Marker()->name());
    setAttribute("Body2MarkerID"s, body2Marker()->name());
    switch (m_Mode)
    {
    case BallJoint::NoStops:
        setAttribute("Mode"s, "NoStops"s);
        break;
    case BallJoint::AMotorUser:
        setAttribute("Mode"s, "AMotorUser"s);
        setAttribute("LowStop0"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamLoStop1), &buf));
        setAttribute("HighStop0"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamHiStop1), &buf));
        setAttribute("LowStop1"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamLoStop2), &buf));
        setAttribute("HighStop1"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamHiStop2), &buf));
        setAttribute("LowStop2"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamLoStop3), &buf));
        setAttribute("HighStop2"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamHiStop3), &buf));
        break;
    case BallJoint::AMotorEuler:
        setAttribute("Mode"s, "AMotorEuler"s);
        setAttribute("LowStop0"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamLoStop1), &buf));
        setAttribute("HighStop0"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamHiStop1), &buf));
        setAttribute("LowStop1"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamLoStop2), &buf));
        setAttribute("HighStop1"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamHiStop2), &buf));
        setAttribute("LowStop2"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamLoStop3), &buf));
        setAttribute("HighStop2"s, *GSUtil::ToString(dJointGetAMotorParam(m_MotorJointID, dParamHiStop3), &buf));
        break;
    }
}

BallJoint::Mode BallJoint::GetMode() const
{
    return m_Mode;
}

std::string BallJoint::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tXP\tYP\tZP\ttheta0\ttheta1\ttheta2\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\tMotorFX1\tMotorFY1\tMotorFZ1\tMotorTX1\tMotorTY1\tMotorTZ1\tMotorFX2\tMotorFY2\tMotorFZ2\tMotorTX2\tMotorTY2\tMotorTZ2\n";
    }
    dVector3 p;
    GetBallAnchor(p);
    double theta0 = dJointGetAMotorAngle(m_MotorJointID, 0);
    double theta1 = dJointGetAMotorAngle(m_MotorJointID, 1);
    double theta2 = dJointGetAMotorAngle(m_MotorJointID, 2);

    ss << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] << "\t" <<
          theta0 << "\t" << theta1 << "\t" << theta2 << "\t" <<
          JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
          JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
          JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
          JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] << "\t" <<
          m_MotorJointFeedback.f1[0] << "\t" << m_MotorJointFeedback.f1[1] << "\t" << m_MotorJointFeedback.f1[2] << "\t" <<
          m_MotorJointFeedback.t1[0] << "\t" << m_MotorJointFeedback.t1[1] << "\t" << m_MotorJointFeedback.t1[2] << "\t" <<
          m_MotorJointFeedback.f2[0] << "\t" << m_MotorJointFeedback.f2[1] << "\t" << m_MotorJointFeedback.f2[2] << "\t" <<
          m_MotorJointFeedback.t2[0] << "\t" << m_MotorJointFeedback.t2[1] << "\t" << m_MotorJointFeedback.t2[2] << "\t" <<
          "\n";
    return ss.str();
}



