/*
 *  SliderJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 25/05/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#include "SliderJoint.h"
#include "Simulation.h"
#include "Body.h"

#include "ode/ode.h"

#include <iostream>
#include <cmath>
#include <sstream>

SliderJoint::SliderJoint(dWorldID worldID) : Joint()
{
    setJointID(dJointCreateSlider(worldID, nullptr));
    dJointSetData(JointID(), this);

    dJointSetFeedback(JointID(), JointFeedback());
}

SliderJoint::~SliderJoint()
{
}

void SliderJoint::SetSliderAxis(double x, double y, double z)
{
    dVector3 v;
    v[0] = x; v[1] = y; v[2] = z;
    dNormalize3(v);
    dJointSetSliderAxis(JointID(), v[0], v[1], v[2]);
}

void SliderJoint::GetSliderAxis(dVector3 result)
{
    dJointGetSliderAxis(JointID(), result);
}

double SliderJoint::GetSliderDistance()
{
    return dJointGetSliderPosition(JointID()) + m_StartDistanceReference;
}

double SliderJoint::GetSliderDistanceRate()
{
    return dJointGetSliderPositionRate(JointID());
}

void SliderJoint::SetStartDistanceReference(double startDistanceReference)
{
    m_StartDistanceReference = startDistanceReference;
}

void SliderJoint::SetJointStops(double loStop, double hiStop)
{
    if (loStop >= hiStop) throw(__LINE__);

    // correct for m_StartDistanceReference
    loStop -= m_StartDistanceReference;
    hiStop -= m_StartDistanceReference;

    // note there is safety feature that stops setting incompatible low and high
    // stops which can cause difficulties. The safe option is to set them twice.

    dJointSetSliderParam(JointID(), dParamLoStop, loStop);
    dJointSetSliderParam(JointID(), dParamHiStop, hiStop);
    dJointSetSliderParam(JointID(), dParamLoStop, loStop);
    dJointSetSliderParam(JointID(), dParamHiStop, hiStop);
}

void SliderJoint::SetStopCFM(double cfm)
{
    dJointSetSliderParam (JointID(), dParamStopCFM, cfm);
}

void SliderJoint::SetStopERP(double erp)
{
    dJointSetSliderParam (JointID(), dParamStopERP, erp);
}

void SliderJoint::SetStopSpringDamp(double springConstant, double dampingConstant, double integrationStep)
{
    double ERP = integrationStep * springConstant/(integrationStep * springConstant + dampingConstant);
    double CFM = 1/(integrationStep * springConstant + dampingConstant);
    SetStopERP(ERP);
    SetStopCFM(CFM);
}

void SliderJoint::SetStopSpringERP(double springConstant, double ERP, double integrationStep)
{
    double CFM = ERP / (integrationStep * springConstant);
    SetStopERP(ERP);
    SetStopCFM(CFM);
}

void SliderJoint::SetStopBounce(double bounce)
{
    dJointSetSliderParam (JointID(), dParamBounce, bounce);
}

void SliderJoint::Update()
{
}

std::string SliderJoint::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tXA\tYA\tZA\tDistance\tDistanceRate\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\n";
    }
    dVector3 a;
    GetSliderAxis(a);

    ss << simulation()->GetTime() << "\t" <<
          a[0] << "\t" << a[1] << "\t" << a[2] << "\t" << GetSliderDistance() << "\t" << GetSliderDistanceRate() << "\t" <<
          JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
          JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
          JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
          JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] << "\t" <<
          "\n";
    return ss.str();
}



