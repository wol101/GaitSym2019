/*
 *  TorqueReporter.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 06/01/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

#include "TorqueReporter.h"
#include "Simulation.h"
#include "Body.h"
#include "Muscle.h"
#include "GSUtil.h"

#include <iostream>
#include <vector>
#include <sstream>

TorqueReporter::TorqueReporter()
{
    mBody = nullptr;
    mAxis = pgd::Vector3(1, 0, 0);
}


void TorqueReporter::SetAxis(double x, double y, double z)
{
    mAxis = pgd::Vector3(x, y, z);
    mAxis.Normalize();
}

//Theory:
//Point = point of force effect
//Center = center of gravity/rotation
//Direction = direction of force effect (unit)
//Force_Scalar = amount of force effect
//Torque = cross(Point - Center, Direction * Force_Scalar)
//Axis = normalize(cross(Point - Center, Direction));
//Torque_Scalar = dot(Point - Center, Cross(Direction, Axis)) * Force_Scalar;

std::string TorqueReporter::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tTension\tWorldTorqueX\tWorldTorqueY\tWorldTorqueZ\tBodyTorqueX\tBodyTorqueY\tBodyTorqueZ\tAxisTorqueX\tAxisTorqueY\tAxisTorqueZ\tWorldMAX\tWorldMAY\tWorldMAZ\tBodyMAX\tBodyMAY\tBodyMAZ\tAxisMAX\tAxisMAY\tAxisMAZ\n";
    }

    // sum the torques acting on body 0 of the joint
    std::vector<std::unique_ptr<PointForce >> *pointForceList = mMuscle->GetPointForceList();
    double tension = mMuscle->GetTension();
    pgd::Vector3 torque, point, force, centre;
    double *forcePoint, *forceDirection;
    pgd::Vector3 totalTorque, momentArm;
    dVector3 result;
    dBodyGetRelPointPos(mBody->GetBodyID(), mPivotPoint.x, mPivotPoint.y, mPivotPoint.z, result); // needs to be in world coordinates
    centre = pgd::Vector3(result[0], result[1], result[2]);

// These are the same but the second option works even when tension is zero
//        if (tension > 0)
//        {
//            for (unsigned int i = 0; i < pointForceList->size(); i++)
//            {
//                if ((*pointForceList)[i]->body == mBody)
//                {
//                    //Torque = cross(Point - Center, Force)
//                    forcePoint = (*pointForceList)[i]->point;
//                    point = pgd::Vector3(forcePoint[0], forcePoint[1], forcePoint[2]);
//                    forceDirection = (*pointForceList)[i]->vector;
//                    force = pgd::Vector3(forceDirection[0], forceDirection[1], forceDirection[2]) * tension;
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
                point = pgd::Vector3(forcePoint[0], forcePoint[1], forcePoint[2]);
                forceDirection = (*pointForceList)[i]->vector;
                force = pgd::Vector3(forceDirection[0], forceDirection[1], forceDirection[2]);
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
    pgd::Vector3 axisBasedTorque = R * totalTorque;
    pgd::Vector3 axisBasedMomentArm = R * momentArm;

    ss << simulation()->GetTime() << "\t" << tension << "\t"
       << totalTorque.x << "\t" << totalTorque.y << "\t" << totalTorque.z << "\t"
       << bodyTorque[0] << "\t" << bodyTorque[1] << "\t" << bodyTorque[2] << "\t"
       << axisBasedTorque.x << "\t" << axisBasedTorque.y << "\t" << axisBasedTorque.z << "\t"
       << momentArm.x << "\t" << momentArm.y << "\t" << momentArm.z << "\t"
       << bodyMomentArm[0] << "\t" << bodyMomentArm[1] << "\t" << bodyMomentArm[2] << "\t"
       << axisBasedMomentArm.x << "\t" << axisBasedMomentArm.y << "\t" << axisBasedMomentArm.z
       << "\n";
    return ss.str();
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
