/*
 *  FloatingHingeJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 30/12/2006.
 *  Copyright 2006 Bill Sellers. All rights reserved.
 *
 */

#include "FloatingHingeJoint.h"

#include "PGDMath.h"
#include "DataFile.h"
#include "Simulation.h"
#include "Body.h"
#include "Marker.h"
#include "GSUtil.h"

#include "ode/ode.h"


#include <iostream>
#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

using namespace std::string_literals;

FloatingHingeJoint::FloatingHingeJoint(dWorldID worldID) : Joint()
{
    setJointID(dJointCreateFloatingHinge(worldID, nullptr));
    dJointSetData(JointID(), this);

    dJointSetFeedback(JointID(), JointFeedback());
}

void FloatingHingeJoint::SetFloatingHingeAxis(double x, double y, double z)
{
    dJointSetFloatingHingeAxis(JointID(), x, y, z);
}

void FloatingHingeJoint::GetFloatingHingeAxis(dVector3 result)
{
    dJointGetFloatingHingeAxis(JointID(), result);
}

double FloatingHingeJoint::GetFloatingHingeAngleRate()
{
    return dJointGetFloatingHingeAngleRate(JointID());
}

void FloatingHingeJoint::SetJointStops(double loStop, double hiStop)
{
    // note there is safety feature that stops setting incompatible low and high
    // stops which can cause difficulties. The safe option is to set them twice.

    dJointSetFloatingHingeParam (JointID(), dParamLoStop, loStop);
    dJointSetFloatingHingeParam (JointID(), dParamHiStop, hiStop);
    dJointSetFloatingHingeParam (JointID(), dParamLoStop, loStop);
    dJointSetFloatingHingeParam (JointID(), dParamHiStop, hiStop);
}

std::string *FloatingHingeJoint::createFromAttributes()
{
    if (Joint::createFromAttributes()) return lastErrorPtr();
    pgd::Vector3 axis = body1Marker()->GetWorldAxis(Marker::Axis::X);
    this->SetFloatingHingeAxis(axis.x, axis.y, axis.z);
    if (CFM() >= 0) dJointSetFloatingHingeParam(JointID(), dParamCFM, CFM());
    if (ERP() >= 0) dJointSetFloatingHingeParam(JointID(), dParamERP, ERP());

    std::string buf;
    if (findAttribute("LowStop"s, &buf) == nullptr) return lastErrorPtr();
    double loStop = GSUtil::GetAngle(buf);
    if (findAttribute("HighStop"s, &buf) == nullptr) return lastErrorPtr();
    double hiStop = GSUtil::GetAngle(buf);
    if (loStop >= hiStop)
    {
        setLastError("FloatingHinge ID=\""s + name() +"\" LowStop >= HighStop"s);
        return lastErrorPtr();
    }
    this->SetJointStops(loStop, hiStop);
    return nullptr;
}

void FloatingHingeJoint::appendToAttributes()
{
    Joint::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "FloatingHinge"s);
    setAttribute("LowStop"s, *GSUtil::ToString(dJointGetFloatingHingeParam(JointID(), dParamLoStop), &buf));
    setAttribute("HighStop"s, *GSUtil::ToString(dJointGetFloatingHingeParam(JointID(), dParamHiStop), &buf));
}


