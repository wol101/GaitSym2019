/*
 *  AMotorJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 07/01/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include "ode/ode.h"

#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

#include "AMotorJoint.h"
#include "Simulation.h"
#include "Body.h"

AMotorJoint::AMotorJoint(dWorldID worldID) : Joint()
{
    setJointID(dJointCreateAMotor(worldID, nullptr));
    dJointSetData(JointID(), this);

    dJointSetFeedback(JointID(), JointFeedback());


}


void AMotorJoint::SetStops(double low, double high)
{
    // note there is safety feature that stops setting incompatible low and high
    // stops which can cause difficulties. The safe option is to set them twice.

    dJointSetAMotorParam(JointID(), dParamLoStop, low);
    dJointSetAMotorParam(JointID(), dParamHiStop, high);

    dJointSetAMotorParam(JointID(), dParamLoStop, low);
    dJointSetAMotorParam(JointID(), dParamHiStop, high);

    // we don't want bouncy stops
    dJointSetAMotorParam(JointID(), dParamBounce, 0);
}

// this is the target velocity for the motor
void AMotorJoint::SetTargetVelocity(double targetVelocity)
{
    dJointSetAMotorParam(JointID(), dParamVel, targetVelocity);
}

// this is the maximum torque the motor can generate to achieve the
// desired velocity. Note this isn't used for stops which can generate
// a *lot* of torque.
// set to zero to turn off the motor bit (again stops still work if set)
void AMotorJoint::SetMaxTorque(double maximumTorque)
{
    dJointSetAMotorParam(JointID(), dParamFMax, maximumTorque);
}


// get the axis angular rate
double AMotorJoint::GetAngleRate()
{
    dVector3 axis;
    dJointGetAMotorAxis(JointID(), 0, axis); // get the world axis orientation

    double avel0;
    if (dJointGetBody (JointID(), 0))
        avel0 = dDOT(axis, dBodyGetAngularVel(dJointGetBody (JointID(), 0)));
    else
        avel0 = 0;

    double avel1 = dDOT(axis, dBodyGetAngularVel(dJointGetBody (JointID(), 1)));

    double rate = avel1 - avel0; // I hope the sign is right here (it is different from dJointGetHingeAngleRate)

    return rate;
}

// this routine sets up the axis for the joint
// axisMode: 0 global, 1 relative to body 1, 2 relative to body 2
// axes are specified either globally, or relative to a body depending on mode
void AMotorJoint::SetAxis(double x, double y, double z, int axisMode)
{
    dVector3 result;

    if (axisMode == 0)
    {
        result[0] = x;
        result[1] = y;
        result[2] = z;
    }
    else
    {
        if (axisMode == 1) // relative to body1
        {
            dBodyVectorToWorld(dJointGetBody(JointID(), 0), x, y, z, result);
        }
        else // relative to body1
        {
            dBodyVectorToWorld(dJointGetBody(JointID(), 1), x, y, z, result);
        }
    }

    dJointSetAMotorMode(JointID(), dAMotorUser);
    dJointSetAMotorNumAxes(JointID(), 1);
    dJointSetAMotorAxis(JointID(), 0, axisMode, result[0], result[1], result[2]);

    // and now must set the angles based on the relative pose of the two bodies
    SetAngle();
}

// parses the axis allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void AMotorJoint::SetAxis(const char *buf)
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
        std::cerr << "Error in AMotorJoint::SetAxis\n";
        return; // error condition
    }

    if (isalpha((int)*lBufPtrs[0]) == 0)
    {
        for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        dBodyGetPosRelPoint(dJointGetBody(JointID(), 0), pos[0], pos[1], pos[2], result); // convert from world to body
        SetAxis(result[0], result[1], result[2], 1);
        return;
    }

    if (count < 4)
    {
        std::cerr << "Error in AMotorJoint::SetAxis\n";
        return; // error condition
    }
    Body *theBody = simulation()->GetBody(lBufPtrs[0]);
    if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            if (dJointGetBody(JointID(), 0))
            {
                dBodyVectorFromWorld(dJointGetBody(JointID(), 0), pos[0], pos[1], pos[2], result); // convert from world to body
                SetAxis(result[0], result[1], result[2], 1);
            }
            else
            {
                SetAxis(pos[0], pos[1], pos[2], 0);
            }
            return;
        }
        else
        {
            std::cerr << "Error in AMotorJoint::SetAxis\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyVectorToWorld(theBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from body to world
    dBodyVectorFromWorld(dJointGetBody(JointID(), 0), result[0], result[1], result[2], pos); // convert from world to body
    SetAxis(pos[0], pos[1], pos[2], 1);
}

// this is the tricky bit. This routine calculates the angle around the axis of the two bodies
// but it needs to match up with the AMotor internal calculations which work in global coordinates
double AMotorJoint::GetAngle()
{
    dVector3 g;
    dJointGetAMotorAxis (JointID(), 0, g); // this routine returns the axis in global coordinates

    const double *body1Rotation;
    dMatrix3 identityR;
    if (dJointGetBody(JointID(), 0))
    {
        body1Rotation = dBodyGetRotation(dJointGetBody(JointID(), 0));
    }
    else
    {
        dRSetIdentity(identityR);
        body1Rotation = identityR;
    }

    const double *body2Rotation = dBodyGetRotation(dJointGetBody(JointID(), 1));

    // now we need to find the rotation about this axis of the two bodies
    // so find a matrix that converts (0, 0, 1) to this axis

    // dNormalize3(globalAxis); // actually this should already normalised
    dVector3 p, q;
    dMatrix3 R;
    // calculate 2 perpendicular vectors
    dPlaneSpace(g, p, q);
    // assemble the matrix
    R[3] = R[7] = R[11] = 0;

    R[0] = p[0]; R[4] = p[1]; R[8] =  p[2];
    R[1] = q[0]; R[5] = q[1]; R[9] =  q[2];
    R[2] = g[0]; R[6] = g[1]; R[10] = g[2];

    // so measure how far the unit X vector is rotated
    p[0] = 1; p[1] = p[2] = 0;
    dVector3 b, a;
    dMULTIPLY0_331(b, body1Rotation, p); // rotate by body rotation
    dMULTIPLY1_331(a, R, b); // rotate to axis coordinate system (using inverse of R)
    double angleBody1 = atan2(a[1], a[0]);
    dMULTIPLY0_331(b, body2Rotation, p); // rotate by body rotation
    dMULTIPLY1_331(a, R, b); // rotate to axis coordinate system (using inverse of R)
    double angleBody2 = atan2(a[1], a[0]);

    double angle = angleBody2 - angleBody1;
    if (angle < -M_PI) angle += (2 * M_PI);
    else if (angle > M_PI) angle -= (2 * M_PI);

    return angle;
}

void AMotorJoint::SetAngle()
{
    double angle = GetAngle();
    dJointSetAMotorAngle(JointID(), 0, angle);
}


void AMotorJoint::Update()
{
    SetAngle();
}

void AMotorJoint::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == nullptr)
        {
            if (GetName().size() == 0) std::cerr << "AMotorJoint::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename.c_str()));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\tARV\tAT\tPower\n";
        }
    }


    if (dumpStream())
    {
        double axisVelocity = GetAngleRate();
        dVector3 axis;
        dJointGetAMotorAxis(JointID(), 0, axis); // get the world axis orientation
        double axisTorque = dDOT(axis, JointFeedback()->t1);

        *dumpStream() << simulation()->GetTime() << "\t" <<
                JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
                JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
                JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
                JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] << "\t" <<
                axisVelocity << "\t" << axisTorque << "\t" << axisVelocity * axisTorque << "\t" <<
                "\n";
    }
}

