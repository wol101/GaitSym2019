/*
 *  UniversalJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 21/12/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

#include "UniversalJoint.h"
#include "DataFile.h"
#include "Body.h"
#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"

#include "ode/ode.h"

#include <sstream>

using namespace std::string_literals;

UniversalJoint::UniversalJoint(dWorldID worldID) : Joint()
{
    setJointID(dJointCreateUniversal(worldID, nullptr));
    dJointSetData(JointID(), this);

    dJointSetFeedback(JointID(), JointFeedback());
}

void UniversalJoint::SetUniversalAnchor(double x, double y, double z)
{
    dJointSetUniversalAnchor (JointID(), x, y, z);
}

void UniversalJoint::SetUniversalAxis1(double x, double y, double z)
{
    dVector3 v;
    v[0] = x; v[1] = y; v[2] = z;
    dNormalize3(v);
    dJointSetUniversalAxis1 (JointID(), v[0], v[1], v[2]);
}

void UniversalJoint::SetUniversalAxis2(double x, double y, double z)
{
    dVector3 v;
    v[0] = x; v[1] = y; v[2] = z;
    dNormalize3(v);
    dJointSetUniversalAxis2 (JointID(), v[0], v[1], v[2]);
}

void UniversalJoint::SetStopCFM1(double cfm)
{
    m_StopCFM1 = cfm;
    dJointSetUniversalParam (JointID(), dParamStopCFM1, cfm);
}

void UniversalJoint::SetStopERP1(double erp)
{
    m_StopERP1 = erp;
    dJointSetUniversalParam (JointID(), dParamStopERP1, erp);
}

void UniversalJoint::SetStopSpringDamp1(double springConstant, double dampingConstant, double integrationStep)
{
    double ERP = integrationStep * springConstant/(integrationStep * springConstant + dampingConstant);
    double CFM = 1/(integrationStep * springConstant + dampingConstant);
    SetStopERP1(ERP);
    SetStopCFM1(CFM);
}

void UniversalJoint::SetStopSpringERP1(double springConstant, double ERP, double integrationStep)
{
    double CFM = ERP / (integrationStep * springConstant);
    SetStopERP1(ERP);
    SetStopCFM1(CFM);
}

void UniversalJoint::SetStopBounce1(double bounce)
{
    dJointSetUniversalParam (JointID(), dParamBounce1, bounce);
}

void UniversalJoint::SetStartAngleReference1(double startAngleReference)
{
    m_StartAngleReference1 = startAngleReference;
}

void UniversalJoint::SetJointStops1(double loStop, double hiStop)
{
    // correct for m_StartAngleReference
    loStop -= m_StartAngleReference1;
    hiStop -= m_StartAngleReference1;

    if (loStop < -M_PI) loStop = -dInfinity;
    if (hiStop > M_PI) hiStop = dInfinity;

    // note there is safety feature that stops setting incompatible low and high
    // stops which can cause difficulties. The safe option is to set them twice.

    dJointSetUniversalParam(JointID(), dParamLoStop1, loStop);
    dJointSetUniversalParam(JointID(), dParamHiStop1, hiStop);
    dJointSetUniversalParam(JointID(), dParamLoStop1, loStop);
    dJointSetUniversalParam(JointID(), dParamHiStop1, hiStop);
}

void UniversalJoint::SetStopCFM2(double cfm)
{
    m_StopCFM2 = cfm;
    dJointSetUniversalParam (JointID(), dParamStopCFM2, cfm);
}

void UniversalJoint::SetStopERP2(double erp)
{
    m_StopERP2 = erp;
    dJointSetUniversalParam (JointID(), dParamStopERP2, erp);
}

void UniversalJoint::SetStopSpringDamp2(double springConstant, double dampingConstant, double integrationStep)
{
    double ERP = integrationStep * springConstant/(integrationStep * springConstant + dampingConstant);
    double CFM = 1/(integrationStep * springConstant + dampingConstant);
    SetStopERP2(ERP);
    SetStopCFM2(CFM);
}

void UniversalJoint::SetStopSpringERP2(double springConstant, double ERP, double integrationStep)
{
    double CFM = ERP / (integrationStep * springConstant);
    SetStopERP2(ERP);
    SetStopCFM2(CFM);
}

void UniversalJoint::SetStopBounce2(double bounce)
{
    dJointSetUniversalParam (JointID(), dParamBounce2, bounce);
}

void UniversalJoint::SetStartAngleReference2(double startAngleReference)
{
    m_StartAngleReference2 = startAngleReference;
}

void UniversalJoint::SetJointStops2(double loStop, double hiStop)
{
    // correct for m_StartAngleReference
    loStop -= m_StartAngleReference2;
    hiStop -= m_StartAngleReference2;

    if (loStop < -M_PI) loStop = -dInfinity;
    if (hiStop > M_PI) hiStop = dInfinity;

    // note there is safety feature that stops setting incompatible low and high
    // stops which can cause difficulties. The safe option is to set them twice.

    dJointSetUniversalParam(JointID(), dParamLoStop2, loStop);
    dJointSetUniversalParam(JointID(), dParamHiStop2, hiStop);
    dJointSetUniversalParam(JointID(), dParamLoStop2, loStop);
    dJointSetUniversalParam(JointID(), dParamHiStop2, hiStop);
}

void UniversalJoint::GetUniversalAnchor(dVector3 result)
{
    dJointGetUniversalAnchor (JointID(), result);
}

void UniversalJoint::GetUniversalAnchor2(dVector3 result)
{
    dJointGetUniversalAnchor2 (JointID(), result);
}

void UniversalJoint::GetUniversalAxis1(dVector3 result)
{
    dJointGetUniversalAxis1 (JointID(), result);
}

void UniversalJoint::GetUniversalAxis2(dVector3 result)
{
    dJointGetUniversalAxis2 (JointID(), result);
}

double UniversalJoint::GetUniversalAngle1()
{
    return dJointGetUniversalAngle1 (JointID()) + m_StartAngleReference1;
}

double UniversalJoint::GetUniversalAngle2()
{
    return dJointGetUniversalAngle2 (JointID()) + m_StartAngleReference1;
}

double UniversalJoint::GetUniversalAngle1Rate()
{
    return dJointGetUniversalAngle1Rate (JointID());
}

double UniversalJoint::GetUniversalAngle2Rate()
{
    return dJointGetUniversalAngle2Rate (JointID());
}

std::string *UniversalJoint::createFromAttributes()
{
    if (Joint::createFromAttributes()) return lastErrorPtr();
    std::string buf;

    pgd::Vector3 position = body1Marker()->GetWorldPosition();
    this->SetUniversalAnchor(position.x, position.y, position.z);
    pgd::Vector3 x, y, z;
    body1Marker()->GetWorldBasis(&x, &y, &z);
    this->SetUniversalAxis1(x.x, x.y, x.z);
    this->SetUniversalAxis2(y.x, y.y, y.z);
    if (CFM() >= 0) dJointSetUniversalParam (JointID(), dParamCFM, CFM());
    if (ERP() >= 0) dJointSetUniversalParam (JointID(), dParamERP, ERP());

    if (findAttribute("LowStop1"s, &buf) == nullptr) return lastErrorPtr();
    double loStop1 = GSUtil::GetAngle(buf);
    if (findAttribute("HighStop1"s, &buf) == nullptr) return lastErrorPtr();
    double hiStop1 = GSUtil::GetAngle(buf);
    if (loStop1 >= hiStop1)
    {
        setLastError("Universal ID=\""s + name() +"\" LowStop1 >= HighStop1"s);
        return lastErrorPtr();
    }
    this->SetJointStops1(loStop1, hiStop1);

    if (findAttribute("LowStop2"s, &buf) == nullptr) return lastErrorPtr();
    double loStop2 = GSUtil::GetAngle(buf);
    if (findAttribute("HighStop2"s, &buf) == nullptr) return lastErrorPtr();
    double hiStop2 = GSUtil::GetAngle(buf);
    if (loStop2 >= hiStop2)
    {
        setLastError("Universal ID=\""s + name() +"\" LowStop2 >= HighStop2"s);
        return lastErrorPtr();
    }
    this->SetJointStops2(loStop2, hiStop2);

    if (findAttribute("StopCFM1"s, &buf))
    {
        this->SetStopCFM1(GSUtil::Double(buf));
        if (findAttribute("StopERP1"s, &buf) == nullptr) return lastErrorPtr();
        this->SetStopERP1(GSUtil::Double(buf));
    }
    if (findAttribute("StopCFM2"s, &buf))
    {
        this->SetStopCFM2(GSUtil::Double(buf));
        if (findAttribute("StopERP2"s, &buf) == nullptr) return lastErrorPtr();
        this->SetStopERP2(GSUtil::Double(buf));
    }

    if (findAttribute("StopBounce1"s, &buf)) this->SetStopBounce1(GSUtil::Double(buf));
    if (findAttribute("StopBounce2"s, &buf)) this->SetStopBounce2(GSUtil::Double(buf));

    return nullptr;
}

void UniversalJoint::appendToAttributes()
{
    Joint::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Universal"s);
    setAttribute("LowStop1"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamLoStop1), &buf));
    setAttribute("HighStop1"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamHiStop1), &buf));
    setAttribute("LowStop2"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamLoStop2), &buf));
    setAttribute("HighStop2"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamHiStop2), &buf));
    if (m_StopCFM1 > 0) setAttribute("StopCFM1"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamStopCFM1), &buf));
    if (m_StopERP1 > 0) setAttribute("StopERP1"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamStopERP1), &buf));
    setAttribute("StopBounce1"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamBounce1), &buf));
    if (m_StopCFM2 > 0) setAttribute("StopCFM2"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamStopCFM2), &buf));
    if (m_StopERP2 > 0) setAttribute("StopERP2"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamStopERP2), &buf));
    setAttribute("StopBounce2"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamBounce2), &buf));
}

std::string UniversalJoint::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tXP\tYP\tZP\tXA1\tYA1\tZA1\tAngle1\tAngleRate1\tXA2\tYA2\tZA2\tAngle2\tAngleRate2\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\n";
    }
    dVector3 p, a1, a2;
    GetUniversalAnchor(p);
    GetUniversalAxis1(a1);
    GetUniversalAxis2(a2);

    ss << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] << "\t" <<
          a1[0] << "\t" << a1[1] << "\t" << a1[2] << "\t" << GetUniversalAngle1() << "\t" << GetUniversalAngle1Rate() << "\t" <<
          a2[0] << "\t" << a2[1] << "\t" << a2[2] << "\t" << GetUniversalAngle2() << "\t" << GetUniversalAngle2Rate() << "\t" <<
          JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
          JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
          JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
          JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] <<
          "\n";
    return ss.str();
}



