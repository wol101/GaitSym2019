/*
 *  TorqueReporter.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 06/01/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

#include "ode/ode.h"
#include <iostream>
#include <vector>

#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

#include "TorqueReporter.h"
#include "Simulation.h"
#include "Body.h"
#include "Muscle.h"
#include "GSUtil.h"

TorqueReporter::TorqueReporter()
{
    mBody = 0;
    mAxis = pgd::Vector(1, 0, 0);
}

// parses the position allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void TorqueReporter::SetPivotPoint(const char *buf)
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
        std::cerr << "Error in TorqueReporter::SetPivotPoint\n";
        return; // error condition
    }

    if (isalpha((int)*lBufPtrs[0]) == 0)
    {
        for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        dBodyGetPosRelPoint(mBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
        SetPivotPoint(result[0], result[1], result[2]);
        return;
    }

    if (count < 4)
    {
        std::cerr << "Error in TorqueReporter::SetPivotPoint\n";
        return; // error condition
    }
    Body *theBody = simulation()->GetBody(lBufPtrs[0]);
    if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            dBodyGetPosRelPoint(mBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
            SetPivotPoint(result[0], result[1], result[2]);
            return;
        }
        else
        {
            std::cerr << "Error in TorqueReporter::SetPivotPoint\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyGetRelPointPos(theBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from body to world
    dBodyGetPosRelPoint(mBody->GetBodyID(), result[0], result[1], result[2], pos); // convert from world to body
    SetPivotPoint(pos[0], pos[1], pos[2]);
}

void TorqueReporter::SetAxis(double x, double y, double z)
{
    mAxis = pgd::Vector(x, y, z);
    mAxis.Normalize();
}

// parses the position allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void TorqueReporter::SetAxis(const char *buf)
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
        std::cerr << "Error in TorqueReporter::SetHingeAxis\n";
        return; // error condition
    }

    if (isalpha((int)*lBufPtrs[0]) == 0)
    {
        for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        SetAxis(pos[0], pos[1], pos[2]);
        return;
    }

    if (count < 4)
    {
        std::cerr << "Error in TorqueReporter::SetHingeAxis\n";
        return; // error condition
    }
    Body *theBody = simulation()->GetBody(lBufPtrs[0]);
    if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            SetAxis(pos[0], pos[1], pos[2]);
            return;
        }
        else
        {
            std::cerr << "Error in TorqueReporter::SetHingeAxis\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyVectorToWorld(theBody->GetBodyID(), pos[0], pos[1], pos[2], result);
    SetAxis(result[0], result[1], result[2]);
}


//Theory:
//Point = point of force effect
//Center = center of gravity/rotation
//Direction = direction of force effect (unit)
//Force_Scalar = amount of force effect
//Torque = cross(Point - Center, Direction * Force_Scalar)
//Axis = normalize(cross(Point - Center, Direction));
//Torque_Scalar = dot(Point - Center, Cross(Direction, Axis)) * Force_Scalar;

void TorqueReporter::Dump()
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
            *dumpStream() << "Time\tTension\tWorldTorqueX\tWorldTorqueY\tWorldTorqueZ\tBodyTorqueX\tBodyTorqueY\tBodyTorqueZ\tAxisTorqueX\tAxisTorqueY\tAxisTorqueZ\tWorldMAX\tWorldMAY\tWorldMAZ\tBodyMAX\tBodyMAY\tBodyMAZ\tAxisMAX\tAxisMAY\tAxisMAZ\n";
        }
    }


    if (dumpStream())
    {
        // sum the torques acting on body 0 of the joint
        std::vector<PointForce *> *pointForceList = mMuscle->GetPointForceList();
        double tension = mMuscle->GetTension();
        pgd::Vector torque, point, force, centre;
        double *forcePoint, *forceDirection;
        pgd::Vector totalTorque, momentArm;
        dVector3 result;
        dBodyGetRelPointPos(mBody->GetBodyID(), mPivotPoint.x, mPivotPoint.y, mPivotPoint.z, result); // needs to be in world coordinates
        centre = pgd::Vector(result[0], result[1], result[2]);

// These are the same but the second option works even when tension is zero
//        if (tension > 0)
//        {
//            for (unsigned int i = 0; i < pointForceList->size(); i++)
//            {
//                if ((*pointForceList)[i]->body == mBody)
//                {
//                    //Torque = cross(Point - Center, Force)
//                    forcePoint = (*pointForceList)[i]->point;
//                    point = pgd::Vector(forcePoint[0], forcePoint[1], forcePoint[2]);
//                    forceDirection = (*pointForceList)[i]->vector;
//                    force = pgd::Vector(forceDirection[0], forceDirection[1], forceDirection[2]) * tension;
//                    torque = (point - centre) ^ force;
//                    totalTorque += torque;
//                }
//            }
//            momentArm = totalTorque / tension;
//        }
//        else
          {
            for (unsigned int i = 0; i < pointForceList->size(); i++)
            {
                if ((*pointForceList)[i]->body == mBody)
                {
                    //Torque = cross(Point - Center, Force)
                    forcePoint = (*pointForceList)[i]->point;
                    point = pgd::Vector(forcePoint[0], forcePoint[1], forcePoint[2]);
                    forceDirection = (*pointForceList)[i]->vector;
                    force = pgd::Vector(forceDirection[0], forceDirection[1], forceDirection[2]);
                    torque = (point - centre) ^ force;
                    momentArm += torque;
                }
            }
            totalTorque = momentArm * tension;
        }

        // convert to body local coordinates
        dVector3 bodyTorque, bodyMomentArm;
        dBodyVectorFromWorld (mBody->GetBodyID(), totalTorque.x, totalTorque.y, totalTorque.z, bodyTorque);
        dBodyVectorFromWorld (mBody->GetBodyID(), momentArm.x, momentArm.y, momentArm.z, bodyMomentArm);

        // now find the rotation axis specific values
        pgd::Matrix3x3 R;
        CalculateRotationFromAxis(mAxis.x, mAxis.y, mAxis.z, &R);
        pgd::Vector axisBasedTorque = R * totalTorque;
        pgd::Vector axisBasedMomentArm = R * momentArm;

        *dumpStream() << simulation()->GetTime() << "\t" << tension << "\t"
                << totalTorque.x << "\t" << totalTorque.y << "\t" << totalTorque.z << "\t"
                << bodyTorque[0] << "\t" << bodyTorque[1] << "\t" << bodyTorque[2] << "\t"
                << axisBasedTorque.x << "\t" << axisBasedTorque.y << "\t" << axisBasedTorque.z << "\t"
                << momentArm.x << "\t" << momentArm.y << "\t" << momentArm.z << "\t"
                << bodyMomentArm[0] << "\t" << bodyMomentArm[1] << "\t" << bodyMomentArm[2] << "\t"
                << axisBasedMomentArm.x << "\t" << axisBasedMomentArm.y << "\t" << axisBasedMomentArm.z
                << "\n";
    }
}

void TorqueReporter::CalculateRotationFromAxis(double x, double y, double z, pgd::Matrix3x3 *R)
{
    // calculate the rotation needed to get the axis pointing the right way
    // axis is assumed to already be normalised
    dVector3 axis;
    axis[0] = x;
    axis[1] = y;
    axis[2] = z;
    dVector3 p, q;
    // calculate 2 perpendicular vectors
    dPlaneSpace(axis, p, q);
    // assemble the matrix
    *R = pgd::Matrix3x3( axis[0], axis[1], axis[2],
                         p[0],    p[1],    p[2],
                         q[0],    q[1],    q[2] );
}
