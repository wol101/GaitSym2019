/*
 *  UniversalJoint.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 21/12/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

#ifndef UniversalJoint_h
#define UniversalJoint_h

#include "Joint.h"

class Marker;

class UniversalJoint: public Joint
{
public:

    UniversalJoint(dWorldID worldID);

    void SetUniversalAnchor (double x, double y, double z);
    void SetUniversalAxis1(double x, double y, double z);
    void SetUniversalAxis2(double x, double y, double z);

    void SetStartAngleReference1(double startAngleReference);
    void SetJointStops1(double loStop, double hiStop);
    void SetStopCFM1(double cfm);
    void SetStopERP1(double erp);
    void SetStopBounce1(double bounce);
    void SetStopSpringDamp1(double springConstant, double dampingConstant, double integrationStep);
    void SetStopSpringERP1(double springConstant, double ERP, double integrationStep);
    void SetStartAngleReference2(double startAngleReference);
    void SetJointStops2(double loStop, double hiStop);
    void SetStopCFM2(double cfm);
    void SetStopERP2(double erp);
    void SetStopBounce2(double bounce);
    void SetStopSpringDamp2(double springConstant, double dampingConstant, double integrationStep);
    void SetStopSpringERP2(double springConstant, double ERP, double integrationStep);

    void GetUniversalAnchor(dVector3 result);
    void GetUniversalAnchor2(dVector3 result);
    void GetUniversalAxis1(dVector3 result);
    void GetUniversalAxis2(dVector3 result);

    double GetUniversalAngle1();
    double GetUniversalAngle1Rate();
    double GetUniversalAngle2();
    double GetUniversalAngle2Rate();

    virtual std::string dumpToString();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    double m_StartAngleReference1 = 0;
    double m_StartAngleReference2 = 0;

    // these values are just used for saving and loading
    double m_StopCFM1 = -1;
    double m_StopERP1 = -1;
    double m_StopCFM2 = -1;
    double m_StopERP2 = -1;

};



#endif
