/*
 *  PositionReporter.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 13/12/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

#ifndef POSITIONREPORTER_H
#define POSITIONREPORTER_H

#include "Reporter.h"
#include "PGDMath.h"

class Body;

class PositionReporter : public Reporter
{
public:

    PositionReporter();

    void SetBody(Body *body) { mBody = body; }
    Body *GetBody() { return mBody; }

    // these functions set the geom position relative to its body
    void SetPosition (double x, double y, double z)
    {
        mPosition.x = x; mPosition.y = y; mPosition.z = z;
    }
    void SetQuaternion(double q0, double q1, double q2, double q3)
    {
        mQuaternion.n = q0;
        mQuaternion.v.x = q1; mQuaternion.v.y = q2; mQuaternion.v.z = q3;
    }
    void SetPosition (const char *buf);
    void SetQuaternion(const char *buf);

    pgd::Vector GetPosition() { return mPosition; }
    pgd::Quaternion GetQuaternion() { return mQuaternion; }
    pgd::Vector GetWorldPosition();
    pgd::Quaternion GetWorldQuaternion();
    pgd::Vector GetWorldVelocity();

    virtual void Dump();

private:

    Body *mBody;
    pgd::Vector mPosition;
    pgd::Quaternion mQuaternion;

};

#endif // POSITIONREPORTER_H
