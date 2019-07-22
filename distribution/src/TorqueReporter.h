/*
 *  TorqueReporter.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 06/01/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

#ifndef TORQUEREPORTER_H
#define TORQUEREPORTER_H

#include "Reporter.h"
#include "PGDMath.h"

class Muscle;
class Joint;
class Body;

class TorqueReporter : public Reporter
{
public:
    TorqueReporter();

    void SetMuscle(Muscle *muscle) { mMuscle = muscle; }
    void SetPivotPoint(double x, double y, double z) { mPivotPoint = pgd::Vector(x, y, z); }
    void SetPivotPoint(const char *buf);
    void SetAxis(double x, double y, double z);
    void SetAxis(const char *buf);
    void SetBody(Body *body) { mBody = body; }

    Muscle *GetMuscle() { return mMuscle; }
    pgd::Vector GetPivotPoint() { return mPivotPoint; }
    Body *GetBody() { return mBody; }

    virtual void Dump();

    // Utility to function to calculate a matrix that lines the x axis up with a supplied vector
    static void CalculateRotationFromAxis(double x, double y, double z, pgd::Matrix3x3 *R);

private:

    Body *mBody;
    pgd::Vector mPivotPoint;
    pgd::Vector mAxis;
    Muscle *mMuscle;


};

#endif // TORQUEREPORTER_H
