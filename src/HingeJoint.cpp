/*
 *  HingeJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "HingeJoint.h"
#include "DataFile.h"
#include "Body.h"
#include "Simulation.h"
#include "PGDMath.h"
#include "Marker.h"
#include "GSUtil.h"

#include "ode/ode.h"

#include <iostream>
#include <cstdlib>
#include <sstream>

using namespace std::string_literals;

HingeJoint::HingeJoint(dWorldID worldID) : Joint()
{
    setJointID(dJointCreateHinge(worldID, nullptr));
    dJointSetData(JointID(), this);
    dJointSetFeedback(JointID(), JointFeedback());
}

void HingeJoint::SetHingeAnchor (double x, double y, double z)
{
    dJointSetHingeAnchor(JointID(), x, y, z);
}

void HingeJoint::SetHingeAxis(double x, double y, double z)
{
    dVector3 v;
    v[0] = x; v[1] = y; v[2] = z;
    dNormalize3(v);
    dJointSetHingeAxis(JointID(), v[0], v[1], v[2]);
}

void HingeJoint::GetHingeAnchor(dVector3 result)
{
    dJointGetHingeAnchor(JointID(), result);
}

void HingeJoint::GetHingeAnchor2(dVector3 result)
{
    dJointGetHingeAnchor2(JointID(), result);
}

void HingeJoint::GetHingeAxis(dVector3 result)
{
    dJointGetHingeAxis(JointID(), result);
}

double HingeJoint::GetHingeAngle()
{
    return dJointGetHingeAngle(JointID());
}

double HingeJoint::GetHingeAngleRate()
{
    return dJointGetHingeAngleRate(JointID());
}

void HingeJoint::SetTorqueLimits(double loStopTorqueLimit, double hiStopTorqueLimit)
{
    m_LoStopTorqueLimit = loStopTorqueLimit;
    m_HiStopTorqueLimit = hiStopTorqueLimit;
}

void HingeJoint::SetJointStops(double loStop, double hiStop)
{
    if (loStop >= hiStop)
    {
        std::cerr << "Error in HingeJoint::SetJointStops loStop >= hiStop\n";
        throw(__LINE__);
    }

    if (loStop < -M_PI) loStop = -dInfinity;
    if (hiStop > M_PI) hiStop = dInfinity;

    // note there is safety feature that stops setting incompatible low and high
    // stops which can cause difficulties. The safe option is to set them twice.

    dJointSetHingeParam(JointID(), dParamLoStop, loStop);
    dJointSetHingeParam(JointID(), dParamHiStop, hiStop);
    dJointSetHingeParam(JointID(), dParamLoStop, loStop);
    dJointSetHingeParam(JointID(), dParamHiStop, hiStop);
}

void HingeJoint::CalculateStopTorque()
{
#ifdef USE_JOINT_ANGLES_FOR_STOP_TORQUE
    // this is the approximate stop torque from the amount past the limit that the joint is
    // it doesn't take into account damping and probably isn't actualy the value used anyway

    double angle = dJointGetHingeAngle(JointID());
    double loStop = dJointGetHingeParam (JointID(), dParamLoStop);
    double hiStop = dJointGetHingeParam (JointID(), dParamHiStop);
    double t = 0; // calculated torque from error
    double ERP, CFM;
    double kp = 0;
    // double kd = 0; // decided not to use damping in these calculations
    if (angle > hiStop)
    {
        ERP = dJointGetHingeParam (JointID(), dParamStopERP);
        CFM = dJointGetHingeParam (JointID(), dParamStopCFM);
        kp = ERP / (CFM * simulation()->GetTimeIncrement());
        //kd = (1 - ERP) / CFM;
        //t = (hiStop - angle) * kp - GetHingeAngleRate() * kd;
        t = (hiStop - angle) * kp;
    }
    else if (angle < loStop)
    {
        ERP = dJointGetHingeParam (JointID(), dParamStopERP);
        CFM = dJointGetHingeParam (JointID(), dParamStopCFM);
        kp = ERP / (CFM * simulation()->GetTimeIncrement());
        //kd = (1 - ERP) / CFM;
        //t = (loStop - angle) * kp - GetHingeAngleRate() * kd;
        t = (loStop - angle) * kp;
    }
#endif

    if (simulation()->GetTime() <= 0)
    {
        m_axisTorque = 0;
        return;
    }


    // now do it properly
    // first of all we need to convert the forces and torques into the joint local coordinate system

    // the force feedback is at the CM for fixed joints
    // first we need to move it to the joint position

    // calculate the offset of the joint anchor from the CM
    dVector3 jointAnchor;
    dJointGetHingeAnchor(JointID(), jointAnchor);
    dBodyID bodyID = dJointGetBody(JointID(), 0);
    pgd::Vector3 worldForceOffset;
    if (bodyID)
    {
        const double *bodyPosition = dBodyGetPosition(bodyID);
        worldForceOffset = pgd::Vector3(jointAnchor[0] - bodyPosition[0], jointAnchor[1] - bodyPosition[1], jointAnchor[2] - bodyPosition[2]);
    }
    else
    {
        worldForceOffset = pgd::Vector3(jointAnchor[0], jointAnchor[1], jointAnchor[2]);
    }

    // now the linear components of JointFeedback() will generate a torque if applied at this position
    // torque = r x f
    pgd::Vector3 forceCM(JointFeedback()->f1[0], JointFeedback()->f1[1], JointFeedback()->f1[2]);
    pgd::Vector3 addedTorque = worldForceOffset ^ forceCM;

    pgd::Vector3 torqueCM(JointFeedback()->t1[0], JointFeedback()->t1[1], JointFeedback()->t1[2]);
    pgd::Vector3 torqueJointAnchor = torqueCM - addedTorque;

    double torqueScalar = torqueJointAnchor.Magnitude();
    if (torqueScalar == 0)
    {
        m_axisTorque = 0;
        return;
    }

    pgd::Vector3 torqueAxis = torqueJointAnchor / torqueScalar;

    // so the torque around the hinge axis should be: torqueScalar * (hingeAxis .dot. torqueAxis)
    dVector3 result;
    dJointGetHingeAxis(JointID(), result);
    pgd::Vector3 hingeAxis(result[0], result[1], result[2]);
    m_axisTorque = torqueScalar * (hingeAxis * torqueAxis);

    if (m_axisTorqueWindow < 2)
    {
        m_axisTorqueMean = m_axisTorque;
    }
    else
    {
        m_axisTorqueIndex++;
        if (m_axisTorqueIndex >= m_axisTorqueWindow) m_axisTorqueIndex = 0;

        m_axisTorqueTotal -= m_axisTorqueList[size_t(m_axisTorqueIndex)];
        m_axisTorqueTotal += m_axisTorque;
        m_axisTorqueList[size_t(m_axisTorqueIndex)] = m_axisTorque;
        m_axisTorqueMean = m_axisTorqueTotal / m_axisTorqueWindow;
    }
}

int HingeJoint::TestLimits()
{
    if (m_axisTorqueMean < m_LoStopTorqueLimit) return -1;
    if (m_axisTorqueMean > m_HiStopTorqueLimit) return 1;
    return 0;
}

void HingeJoint::SetStopTorqueWindow(int window)
{
    if (window > 1)
    {
        m_axisTorqueList.resize(size_t(window)); // () initialises the array to zero
        m_axisTorqueWindow = window;
    }
    else
    {
        m_axisTorqueList.clear();
        m_axisTorqueWindow = 0;
    }
    m_axisTorqueTotal = 0;
    m_axisTorqueMean = 0;
    m_axisTorqueIndex = 0;
}

void HingeJoint::SetStopCFM(double cfm)
{
    m_StopCFM = cfm;
    dJointSetHingeParam (JointID(), dParamStopCFM, cfm);
}

void HingeJoint::SetStopERP(double erp)
{
    m_StopERP = erp;
    dJointSetHingeParam (JointID(), dParamStopERP, erp);
}

void HingeJoint::SetStopSpringDamp(double springConstant, double dampingConstant, double integrationStep)
{
    double ERP = integrationStep * springConstant/(integrationStep * springConstant + dampingConstant);
    double CFM = 1/(integrationStep * springConstant + dampingConstant);
    SetStopERP(ERP);
    SetStopCFM(CFM);
}

void HingeJoint::SetStopSpringERP(double springConstant, double ERP, double integrationStep)
{
    double CFM = ERP / (integrationStep * springConstant);
    SetStopERP(ERP);
    SetStopCFM(CFM);
}

void HingeJoint::SetStopBounce(double bounce)
{
    dJointSetHingeParam (JointID(), dParamBounce, bounce);
}

void HingeJoint::Update()
{
    CalculateStopTorque();
}

std::string *HingeJoint::createFromAttributes()
{
    if (Joint::createFromAttributes()) return lastErrorPtr();
    std::string buf;

    pgd::Vector3 axis = body1Marker()->GetWorldAxis(Marker::Axis::X);
    this->SetHingeAxis(axis.x, axis.y, axis.z);
    pgd::Vector3 position = body1Marker()->GetWorldPosition();
    this->SetHingeAnchor(position.x, position.y, position.z);
    if (CFM() >= 0) dJointSetHingeParam (JointID(), dParamCFM, CFM());
    if (ERP() >= 0) dJointSetHingeParam (JointID(), dParamERP, ERP());

    if (findAttribute("LowStop"s, &buf) == nullptr) return lastErrorPtr();
    double loStop = GSUtil::GetAngle(buf);
    if (findAttribute("HighStop"s, &buf) == nullptr) return lastErrorPtr();
    double hiStop = GSUtil::GetAngle(buf);
    if (loStop >= hiStop)
    {
        setLastError("Hinge ID=\""s + name() +"\" LowStop >= HighStop"s);
        return lastErrorPtr();
    }
    this->SetJointStops(loStop, hiStop);

    if (findAttribute("HighStopTorqueLimit"s, &buf))
    {
        double hiStopTorqueLimit = GSUtil::Double(buf);
        if (findAttribute("LowStopTorqueLimit"s, &buf) == nullptr) return lastErrorPtr();
        double loStopTorqueLimit = GSUtil::Double(buf);
        this->SetTorqueLimits(loStopTorqueLimit, hiStopTorqueLimit);
        if (findAttribute("StopTorqueWindow"s, &buf) == nullptr) return lastErrorPtr();
        this->SetStopTorqueWindow(GSUtil::Int(buf));
    }

    if (findAttribute("StopCFM"s, &buf)) this->SetStopCFM(GSUtil::Double(buf));
    if (findAttribute("StopERP"s, &buf)) this->SetStopERP(GSUtil::Double(buf));
    if (findAttribute("StopBounce"s, &buf)) this->SetStopBounce(GSUtil::Double(buf));

    return nullptr;
}

void HingeJoint::appendToAttributes()
{
    Joint::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Hinge"s);
    setAttribute("LowStop"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamLoStop), &buf));
    setAttribute("HighStop"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamHiStop), &buf));
    if (m_HiStopTorqueLimit != dInfinity)
    {
        setAttribute("HighStopTorqueLimit"s, *GSUtil::ToString(m_HiStopTorqueLimit, &buf));
        setAttribute("LowStopTorqueLimit"s, *GSUtil::ToString(m_LoStopTorqueLimit, &buf));
        setAttribute("StopTorqueWindow"s, *GSUtil::ToString(m_axisTorqueWindow, &buf));
    }
    if (m_StopCFM >= 0) setAttribute("StopCFM"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamStopCFM), &buf));
    if (m_StopERP >= 0) setAttribute("StopERP"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamStopERP), &buf));
    setAttribute("StopBounce"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamBounce), &buf));
}

std::string HingeJoint::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tXP\tYP\tZP\tXP2\tYP2\tZP2\tXA\tYA\tZA\tAngle\tAngleRate\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\tStopTorque\n";
    }
    dVector3 p, p2, a;
    GetHingeAnchor(p);
    GetHingeAnchor2(p2);
    GetHingeAxis(a);

    ss << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] << "\t" <<
          p2[0] << "\t" << p2[1] << "\t" << p2[2] << "\t" <<
          a[0] << "\t" << a[1] << "\t" << a[2] << "\t" << GetHingeAngle() << "\t" << GetHingeAngleRate() << "\t" <<
          JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
          JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
          JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
          JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] << "\t" <<
          m_axisTorque <<
          "\n";
    return ss.str();
}



