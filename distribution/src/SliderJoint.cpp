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
#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

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

// parses the position allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void SliderJoint::SetSliderAxis(const char *buf)
{
    int i;
    int l = strlen(buf);
    char *lBuf = (char *)alloca((l + 1) * sizeof(char));
    char **lBufPtrs = (char **)alloca(l * sizeof(char *));
    dVector3 pos, result;

    strcpy(lBuf, buf);
    int count = DataFile::ReturnTokens(lBuf, lBufPtrs, l);
    if (count < 3)
    {
        std::cerr << "Error in SliderJoint::SetSliderAxis\n";
        return; // error condition
    }

    if (isalpha((int)*lBufPtrs[0]) == 0)
    {
        for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        SetSliderAxis(pos[0], pos[1], pos[2]);
        return;
    }

    if (count < 4)
    {
        std::cerr << "Error in SliderJoint::SetSliderAxis\n";
        return; // error condition
    }
    Body *theBody = simulation()->GetBody(lBufPtrs[0]);
    if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            SetSliderAxis(pos[0], pos[1], pos[2]);
            return;
        }
        else
        {
            std::cerr << "Error in SliderJoint::SetSliderAxis\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyVectorToWorld(theBody->GetBodyID(), pos[0], pos[1], pos[2], result);
    SetSliderAxis(result[0], result[1], result[2]);
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

    // we don't want bouncy stops
    dJointSetSliderParam(JointID(), dParamBounce, 0);
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

void SliderJoint::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == 0)
        {
            if (GetName().size() == 0) std::cerr << "SliderJoint::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tXA\tYA\tZA\tDistance\tDistanceRate\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\n";
        }
    }


    if (dumpStream())
    {
        dVector3 a;
        GetSliderAxis(a);

        *dumpStream() << simulation()->GetTime() << "\t" <<
                         a[0] << "\t" << a[1] << "\t" << a[2] << "\t" << GetSliderDistance() << "\t" << GetSliderDistanceRate() << "\t" <<
                         JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
                         JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
                         JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
                         JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] << "\t" <<
                         "\n";
    }
}


