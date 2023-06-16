/*
 *  LMotorJoint.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 23/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef LMOTORJOINT_H
#define LMOTORJOINT_H

#include "Joint.h"
#include "PGDMath.h"

class LMotorJoint: public Joint
{
public:

    LMotorJoint(dWorldID worldID);

    void SetNumAxes(int numAxes);
    void SetStops(int anum, double low, double high);
    void SetTargetVelocity(int anum, double targetVelocity);
    void SetTargetPosition(int anum, double targetPosition);
    void SetTargetPositionGain(int anum, double targetPositionGain);
    void SetMaxForce(int anum, double maximumForce);
#ifdef EXPERIMENTAL
    void SetDynamicFriction(double dynamicFrictionIntercept, double dynamicFrictionSlope);
#endif

    int GetNumAxes();
    pgd::Vector3 GetAxis(int anum);
    double GetPosition(int anum);
    double GetPositionRate(int anum);
    void GetPositions(double *x, double *y, double *z);
    double GetTargetPosition(int anum);

    virtual std::string dumpToString();
    virtual void Update();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    void SetPosition(int anum, double position, double time);

    double m_lastPosition0 = 0;
    double m_lastPosition1 = 0;
    double m_lastPosition2 = 0;
    double m_lastPositionRate0 = 0;
    double m_lastPositionRate1 = 0;
    double m_lastPositionRate2 = 0;
    double m_lastTime0 =0;
    double m_lastTime1 = 0;
    double m_lastTime2 = 0;
    bool m_lastTimeValid0 = false;
    bool m_lastTimeValid1 = false;
    bool m_lastTimeValid2 = false;
    bool m_stopsSet0 = false;
    bool m_stopsSet1 = false;
    bool m_stopsSet2 = false;
    double m_targetPosition0 = 0;
    double m_targetPosition1 = 0;
    double m_targetPosition2 = 0;
    bool m_targetPositionSet0 = false;
    bool m_targetPositionSet1 = false;
    bool m_targetPositionSet2 = false;
    double m_targetPositionGain0 = 1;
    double m_targetPositionGain1 = 1;
    double m_targetPositionGain2 = 1;
#ifdef EXPERIMENTAL
    void SetDynamicFriction();
    double m_dynamicFrictionIntercept = 0;
    double m_dynamicFrictionSlope = 0;
    bool m_dynamicFrictionFlag = false;
#endif
};


#endif // LMOTORJOINT_H
