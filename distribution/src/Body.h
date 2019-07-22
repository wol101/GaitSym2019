/*
 *  Body.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 19/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// this class is a wrapper for the ODE body

#ifndef Body_h
#define Body_h

#include "NamedObject.h"
#include "Simulation.h"
#include "PGDMath.h"

class FacetedObject;

class Body: public NamedObject
{
public:

    Body(dWorldID worldID);
    virtual ~Body();

    virtual void LateInitialisation();

    enum LimitTestResult
    {
        WithinLimits = 0,
        XPosError = 1,
        YPosError = 2,
        ZPosError = 3,
        XVelError = 4,
        YVelError = 5,
        ZVelError = 6,
        NumericalError = 7
    };

    enum DragControl
    {
        NoDrag = 0,
        DragCoefficients = 1,
        DragCylinderX = 2,
        DragCylinderY = 3,
        DragCylinderZ = 4
    };

    void SetConstructionPosition(double x, double y, double z) { m_constructionPosition[0] = x; m_constructionPosition[1] = y; m_constructionPosition[2] = z; }
    const double *GetConstructionPosition() { return m_constructionPosition; }
    void SetConstructionDensity(double constructionDensity) { m_constructionDensity = constructionDensity; }
    double GetConstructionDensity() { return m_constructionDensity; }

    void SetPosition(double x, double y, double z);
    void SetQuaternion(double q0, double q1, double q2, double q3);
    std::string *SetPosition(const std::string &buf);
    std::string *SetQuaternion(const std::string &buf);
    void SetLinearVelocity(double x, double y, double z);
    void SetAngularVelocity(double x, double y, double z);
    std::string *SetLinearVelocity(const std::string &buf);
    std::string *SetAngularVelocity(const std::string &buf);

    void SetMass(const dMass *mass);

    void SetPositionLowBound(double x, double y, double z) { m_positionLowBound[0] = x; m_positionLowBound[1] = y; m_positionLowBound[2] = z; }
    void SetPositionHighBound(double x, double y, double z) { m_positionHighBound[0] = x; m_positionHighBound[1] = y; m_positionHighBound[2] = z; }
    void SetLinearVelocityLowBound(double x, double y, double z) { m_linearVelocityLowBound[0] = x; m_linearVelocityLowBound[1] = y; m_linearVelocityLowBound[2] = z; }
    void SetLinearVelocityHighBound(double x, double y, double z) { m_linearVelocityHighBound[0] = x; m_linearVelocityHighBound[1] = y; m_linearVelocityHighBound[2] = z; }
    const double *GetPositionLowBound() { return m_positionLowBound; }
    const double *GetPositionHighBound() { return m_positionHighBound; }
    const double *GetLinearVelocityLowBound() { return m_linearVelocityLowBound; }
    const double *GetLinearVelocityHighBound() { return m_linearVelocityHighBound; }

    void SetLinearDamping(double linearDamping);
    void SetAngularDamping(double angularDamping);
    void SetLinearDampingThreshold(double linearDampingThreshold);
    void SetAngularDampingThreshold(double angularDampingThreshold);
    void SetMaxAngularSpeed(double maxAngularSpeed);


    const double *GetPosition();
    const double *GetQuaternion();
    const double *GetRotation();
    const double *GetLinearVelocity();
    const double *GetAngularVelocity();
    void GetRelativePosition(Body *rel, pgd::Vector *pos);
    void GetRelativeQuaternion(Body *rel, pgd::Quaternion *quat);
    void GetRelativeRotation(Body *rel, pgd::Matrix3x3 *rot);
    void GetRelativeLinearVelocity(Body *rel, pgd::Vector *vel);
    void GetRelativeAngularVelocity(Body *rel, pgd::Vector *rVel);
    double GetMass();
    void GetMass(dMass *mass);
    double GetLinearKineticEnergy();
    void GetLinearKineticEnergy(dVector3 ke);
    double GetRotationalKineticEnergy();
    double GetGravitationalPotentialEnergy();

    dBodyID GetBodyID() { return m_bodyID; }

    LimitTestResult TestLimits();
    int SanityCheck(Body *otherBody, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    void EnterConstructionMode();
    void EnterRunMode();


    // Utility
    void ParallelAxis(dMass *massProperties, const double *translation, const double *quaternion, dMass *newMassProperties);
    static void ParallelAxis(double x, double y, double z, // transformation from centre of mass to new location (m)
                             double mass, // mass (kg)
                             double ixx, double iyy, double izz, double ixy, double iyz, double izx, // moments of inertia kgm2
                             double ang, // rotation angle (radians)
                             double ax, double ay, double az, // axis of rotation - must be unit length
                             double *ixxp, double *iyyp, double *izzp, double *ixyp, double *iyzp, double *izxp); // transformed moments of inertia about new coordinate system

    // these values are really only used for graphics
//    void SetGraphicOffset(double x, double y, double z) {m_graphicOffset[0] = x; m_graphicOffset[1] = y; m_graphicOffset[2] = z; }
//    const double *GetGraphicOffset() { return m_graphicOffset; }
//    void SetGraphicScale(double x, double y, double z) {m_graphicScale[0] = x; m_graphicScale[1] = y; m_graphicScale[2] = z; }
//    const double *GetGraphicScale() { return m_graphicScale; }
//    void SetGraphicFile(const std::string &graphicFile) { m_graphicFile = graphicFile; }
//    std::string GetGraphicFile() { return m_graphicFile; }

    void SetGraphicFile1(const std::string &graphicFile) { m_graphicFile1 = graphicFile; }
    std::string GetGraphicFile1() { return m_graphicFile1; }
    void SetGraphicFile2(const std::string &graphicFile) { m_graphicFile2 = graphicFile; }
    std::string GetGraphicFile2() { return m_graphicFile2; }
    void SetGraphicFile3(const std::string &graphicFile) { m_graphicFile3 = graphicFile; }
    std::string GetGraphicFile3() { return m_graphicFile3; }

    virtual void Dump();
    virtual std::string *CreateFromAttributes();
    virtual void SaveToAttributes();
    virtual void AppendToAttributes();

private:

    dWorldID m_worldID = nullptr;
    dBodyID m_bodyID = nullptr;
    dVector3 m_constructionPosition = {0, 0, 0, 0};
    dQuaternion m_constructionQuaternion = {1, 0, 0, 0};
    double m_constructionDensity = 1000.0;

    dVector3 m_positionLowBound = {-DBL_MAX, -DBL_MAX, -DBL_MAX, 0};
    dVector3 m_positionHighBound = {DBL_MAX, DBL_MAX, DBL_MAX, 0};
    dVector3 m_linearVelocityLowBound = {-DBL_MAX, -DBL_MAX, -DBL_MAX, 0};
    dVector3 m_linearVelocityHighBound = {DBL_MAX, DBL_MAX, DBL_MAX, 0};

    dVector3 m_initialPosition = {0, 0, 0, 0};
    dQuaternion m_initialQuaternion = {1, 0, 0, 0};

    std::string m_graphicFile1;
    std::string m_graphicFile2;
    std::string m_graphicFile3;

    // values used for saving loading only
    double m_LinearDamping = -1;
    double m_AngularDamping = -1;
    double m_LinearDampingThreshold = -1;
    double m_AngularDampingThreshold = -1;
    double m_MaxAngularSpeed = -1;



};

#endif
