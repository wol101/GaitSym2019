/*
 *  UniversalJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 21/12/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

// Note this joint is implemented as an ODE Hinge2 joint so that the requirement for perpendicular joint axes is relaxed

#include "UniversalJoint.h"
#include "DataFile.h"
#include "Body.h"
#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"

#include "ode/ode.h"



#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

using namespace std::string_literals;

UniversalJoint::UniversalJoint(dWorldID worldID) : Joint()
{
    setJointID(dJointCreateUniversal(worldID, nullptr));
    dJointSetData(JointID(), this);

    dJointSetFeedback(JointID(), JointFeedback());
}

void UniversalJoint::LateInitialisation()
{
//    pgd::Vector position = body1Marker()->GetWorldPosition();
//    this->SetUniversalAnchor(position.x, position.y, position.z);
//    pgd::Vector x, y, z;
//    body1Marker()->GetWorldBasis(&x, &y, &z);
//    this->SetUniversalAxis1(x.x, x.y, x.z);
//    this->SetUniversalAxis2(y.x, y.y, y.z);
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
    dJointSetUniversalParam (JointID(), dParamStopCFM1, cfm);
}

void UniversalJoint::SetStopERP1(double erp)
{
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

    // we don't want bouncy stops
    dJointSetUniversalParam(JointID(), dParamBounce1, 0);
}

void UniversalJoint::SetStopCFM2(double cfm)
{
    dJointSetUniversalParam (JointID(), dParamStopCFM2, cfm);
}

void UniversalJoint::SetStopERP2(double erp)
{
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

    // we don't want bouncy stops
    dJointSetUniversalParam(JointID(), dParamBounce2, 0);
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

std::string *UniversalJoint::CreateFromAttributes()
{
    if (Joint::CreateFromAttributes()) return lastErrorPtr();
    std::string buf;

    pgd::Vector position = body1Marker()->GetWorldPosition();
    this->SetUniversalAnchor(position.x, position.y, position.z);
    pgd::Vector x, y, z;
    body1Marker()->GetWorldBasis(&x, &y, &z);
    this->SetUniversalAxis1(x.x, x.y, x.z);
    this->SetUniversalAxis2(y.x, y.y, y.z);
    if (CFM() >= 0) dJointSetUniversalParam (JointID(), dParamCFM, CFM());
    if (ERP() >= 0) dJointSetUniversalParam (JointID(), dParamERP, ERP());

    if (GetAttribute("LowStop1"s, &buf) == nullptr) return lastErrorPtr();
    double loStop1 = GSUtil::GetAngle(buf);
    if (GetAttribute("HighStop1"s, &buf) == nullptr) return lastErrorPtr();
    double hiStop1 = GSUtil::GetAngle(buf);
    if (loStop1 >= hiStop1)
    {
        setLastError("Universal ID=\""s + GetName() +"\" LowStop1 >= HighStop1"s);
        return lastErrorPtr();
    }
    this->SetJointStops1(loStop1, hiStop1);

    if (GetAttribute("LowStop2"s, &buf) == nullptr) return lastErrorPtr();
    double loStop2 = GSUtil::GetAngle(buf);
    if (GetAttribute("HighStop2"s, &buf) == nullptr) return lastErrorPtr();
    double hiStop2 = GSUtil::GetAngle(buf);
    if (loStop2 >= hiStop2)
    {
        setLastError("Universal ID=\""s + GetName() +"\" LowStop2 >= HighStop2"s);
        return lastErrorPtr();
    }
    this->SetJointStops2(loStop2, hiStop2);

    if (GetAttribute("StopCFM1"s, &buf))
    {
        this->SetStopCFM1(GSUtil::Double(buf));
        if (GetAttribute("StopERP1"s, &buf) == nullptr) return lastErrorPtr();
        this->SetStopERP1(GSUtil::Double(buf));
    }
    if (GetAttribute("StopCFM2"s, &buf))
    {
        this->SetStopCFM2(GSUtil::Double(buf));
        if (GetAttribute("StopERP2"s, &buf) == nullptr) return lastErrorPtr();
        this->SetStopERP2(GSUtil::Double(buf));
    }

    if (GetAttribute("StopBounce1"s, &buf)) this->SetStopBounce1(GSUtil::Double(buf));
    if (GetAttribute("StopBounce2"s, &buf)) this->SetStopBounce2(GSUtil::Double(buf));

    return nullptr;
}

void UniversalJoint::AppendToAttributes()
{
    Joint::AppendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Universal"s);
    setAttribute("LowStop1"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamLoStop1), &buf));
    setAttribute("HighStop1"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamHiStop1), &buf));
    setAttribute("LowStop2"s, *GSUtil::ToString(dJointGetUniversalParam(JointID(), dParamLoStop2), &buf));
    setAttribute("HighStop2"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamHiStop2), &buf));
    setAttribute("StopCFM1"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamStopCFM1), &buf));
    setAttribute("StopERP1"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamStopERP1), &buf));
    setAttribute("StopBounce1"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamBounce1), &buf));
    setAttribute("StopCFM2"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamStopCFM2), &buf));
    setAttribute("StopERP2"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamStopERP2), &buf));
    setAttribute("StopBounce2"s, *GSUtil::ToString(dJointGetHingeParam(JointID(), dParamBounce2), &buf));
}

void UniversalJoint::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == nullptr)
        {
            if (GetName().size() == 0) std::cerr << "NamedObject::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tXP\tYP\tZP\tXA1\tYA1\tZA1\tAngle1\tAngleRate1\tXA2\tYA2\tZA2\tAngle2\tAngleRate2\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\n";
        }
    }


    if (dumpStream())
    {
        dVector3 p, a1, a2;
        GetUniversalAnchor(p);
        GetUniversalAxis1(a1);
        GetUniversalAxis2(a2);

        *dumpStream() << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] << "\t" <<
                a1[0] << "\t" << a1[1] << "\t" << a1[2] << "\t" << GetUniversalAngle1() << "\t" << GetUniversalAngle1Rate() << "\t" <<
                a2[0] << "\t" << a2[1] << "\t" << a2[2] << "\t" << GetUniversalAngle2() << "\t" << GetUniversalAngle2Rate() << "\t" <<
                JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
                JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
                JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
                JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] <<
                "\n";
    }
}

