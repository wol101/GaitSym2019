/*
 *  AMotorJoint.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 07/01/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#ifndef AMOTORJOINT_H
#define AMOTORJOINT_H

#include "Joint.h"
#include "PGDMath.h"

class AMotorJoint: public Joint
{
public:

    AMotorJoint(dWorldID worldID);

    void GetAxisAngle(double *xa, double *ya, double *za, double *angle) const;
    pgd::Quaternion GetQuaternion() const;
    pgd::Vector3 GetEulerAngles() const;
    pgd::Vector3 GetEulerAngles(const Marker &basisMarker) const;

    void SetTargetAngles(double angle0);
    void SetTargetAngles(double angle0, double angle1);
    void SetTargetAngles(double angle0, double angle1, double angle2);
    void SetTargetAngleGain(double targetAngleGain);
    void SetMaxTorque(double maxTorque);

    pgd::Vector3 GetTargetAxis() const;
    double GetTargetAngle() const;
    double GetTargetAngleGain() const;
    double GetMaxTorque() const;


#ifdef EXPERIMENTAL
    void SetDynamicFriction(double dynamicFrictionIntercept, double dynamicFrictionSlope);
#endif

    virtual std::string dumpToString();
    virtual void Update();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    const std::vector<double> &targetAnglesList() const;

    bool reverseBodyOrderInCalculations() const;
    void setReverseBodyOrderInCalculations(bool reverseBodyOrderInCalculations);

private:

    pgd::Vector3 m_targetAxis;
    double m_targetAngle = 0;
    double m_targetAngleGain = 1;
    std::vector<double> m_targetAnglesList;
    pgd::Vector3 m_lastDeltaAxis;
    pgd::Vector3 m_deltaAxis;
    double m_deltaAngle = 0;
    pgd::Quaternion m_currentQuaternion;
    pgd::Quaternion m_lastQuaternion;
    pgd::Quaternion m_lastToCurrent;
    bool m_firstTime = true;
    bool m_reverseBodyOrderInCalculations = false;

#ifdef EXPERIMENTAL
    void UpdateDynamicFriction();
    double m_dynamicFrictionIntercept = 0;
    double m_dynamicFrictionSlope = 0;
    bool m_dynamicFrictionFlag = false;
#endif
};


#endif // AMOTORJOINT_H
