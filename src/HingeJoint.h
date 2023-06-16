/*
 *  HingeJoint.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef HingeJoint_h
#define HingeJoint_h

#include "Joint.h"

class Marker;

class HingeJoint: public Joint
{
public:

    HingeJoint(dWorldID worldID);

    void SetHingeAnchor(double x, double y, double z);
    void SetHingeAxis(double x, double y, double z);

    void SetJointStops(double loStop, double hiStop);
    void SetStopCFM(double cfm);
    void SetStopERP(double erp);
    void SetStopBounce(double bounce);
    void SetStopSpringDamp(double springConstant, double dampingConstant, double integrationStep);
    void SetStopSpringERP(double springConstant, double ERP, double integrationStep);

    void GetHingeAnchor(dVector3 result);
    void GetHingeAnchor2(dVector3 result);
    void GetHingeAxis(dVector3 result);

    double GetHingeAngle();
    double GetHingeAngleRate();

    void SetTorqueLimits(double loStopTorqueLimit, double hiStopTorqueLimit);
    int TestLimits();
    void SetStopTorqueWindow(int window);

    virtual void Update();
    virtual std::string dumpToString();
    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    void CalculateStopTorque();

    double m_HiStopTorqueLimit = dInfinity;
    double m_LoStopTorqueLimit = -dInfinity;
    double m_axisTorque = 0;

    std::vector<double> m_axisTorqueList;
    double m_axisTorqueTotal = 0;
    double m_axisTorqueMean = 0;
    int m_axisTorqueIndex = 0;
    int m_axisTorqueWindow = 0;

    // these values are just used for saving and loading
    double m_StopCFM = -1;
    double m_StopERP = -1;

};



#endif
