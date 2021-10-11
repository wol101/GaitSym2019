/*
 *  BallJoint.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/12/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#ifndef BallJoint_h
#define BallJoint_h

#include "Joint.h"
#include "PGDMath.h"
#include "SmartEnum.h"

class Marker;

class BallJoint: public Joint
{
public:

    SMART_ENUM(Mode, modeStrings, modeCount, AMotorUser, AMotorEuler, NoStops);
//    enum Mode { AMotorUser = dAMotorUser, AMotorEuler = dAMotorEuler, NoStops };

    BallJoint(dWorldID worldID, Mode mode);


    void SetBallAnchor (double x, double y, double z);
    void SetStops(double a0Low, double a0High, double a1Low, double a1High, double a2Low, double a2High);
    void SetAxes(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2, int axisMode);
//    void SetAngles();
    void SetEulerReferenceVectors(dVector3 reference1, dVector3 reference2);

    void GetBallAnchor(dVector3 result);
    void GetBallAnchor2(dVector3 result);
    void GetEulerReferenceVectors(dVector3 reference1, dVector3 reference2);
    pgd::Quaternion GetQuaternion();


//    virtual void Update();
    virtual std::string dumpToString();

    // overridden method
    virtual void Attach(Body *body1, Body* body2);
    virtual void Attach();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    Mode GetMode() const;

private:

    dJointID m_MotorJointID = {};
    dJointFeedback m_MotorJointFeedback = {};
    Mode m_Mode = NoStops;

};



#endif
