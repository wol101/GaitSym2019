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
#include "SmartEnum.h"

class Body: public NamedObject
{
public:

    Body(dWorldID worldID);
    virtual ~Body() override;

    virtual void LateInitialisation();

    SMART_ENUM(LimitTestResult, limitTestResultStrings, limitTestResultCount,
               WithinLimits, XPosError, YPosError, ZPosError, XVelError, YVelError, ZVelError, XAVelError, YAVelError, ZAVelError, NumericalError);

    SMART_ENUM(DragControl, dragControlStrings, dragControlCount,
               NoDrag, DragCoefficients, DragCylinderX, DragCylinderY, DragCylinderZ);

    void SetConstructionPosition(double x, double y, double z) { m_constructionPosition[0] = x; m_constructionPosition[1] = y; m_constructionPosition[2] = z; }
    const double *GetConstructionPosition() const { return m_constructionPosition; }
    void SetConstructionDensity(double constructionDensity) { m_constructionDensity = constructionDensity; }
    double GetConstructionDensity() const { return m_constructionDensity; }

    void SetPosition(double x, double y, double z);
    void SetQuaternion(double n, double x, double y, double z);
    std::string *SetPosition(const std::string &buf);
    std::string *SetQuaternion(const std::string &buf);
    void SetLinearVelocity(double x, double y, double z);
    void SetAngularVelocity(double x, double y, double z);
    std::string *SetLinearVelocity(const std::string &buf);
    std::string *SetAngularVelocity(const std::string &buf);
    void SetPositionDelta(double x, double y, double z);
    void SetQuaternionDelta(double n, double x, double y, double z);

    void SetMass(const dMass *mass);

    void SetPositionLowBound(double x, double y, double z) { m_positionLowBound[0] = x; m_positionLowBound[1] = y; m_positionLowBound[2] = z; }
    void SetPositionHighBound(double x, double y, double z) { m_positionHighBound[0] = x; m_positionHighBound[1] = y; m_positionHighBound[2] = z; }
    void SetLinearVelocityLowBound(double x, double y, double z) { m_linearVelocityLowBound[0] = x; m_linearVelocityLowBound[1] = y; m_linearVelocityLowBound[2] = z; }
    void SetLinearVelocityHighBound(double x, double y, double z) { m_linearVelocityHighBound[0] = x; m_linearVelocityHighBound[1] = y; m_linearVelocityHighBound[2] = z; }
    void SetAngularVelocityLowBound(double x, double y, double z) { m_angularVelocityLowBound[0] = x; m_angularVelocityLowBound[1] = y; m_angularVelocityLowBound[2] = z; }
    void SetAngularVelocityHighBound(double x, double y, double z) { m_angularVelocityHighBound[0] = x; m_angularVelocityHighBound[1] = y; m_angularVelocityHighBound[2] = z; }
    const double *GetPositionLowBound() const { return m_positionLowBound; }
    const double *GetPositionHighBound() const { return m_positionHighBound; }
    const double *GetLinearVelocityLowBound() const { return m_linearVelocityLowBound; }
    const double *GetLinearVelocityHighBound() const { return m_linearVelocityHighBound; }
    const double *GetAngularVelocityLowBound() const { return m_angularVelocityLowBound; }
    const double *GetAngularVelocityHighBound() const { return m_angularVelocityHighBound; }

    void SetLinearDamping(double linearDamping);
    void SetAngularDamping(double angularDamping);
    void SetLinearDampingThreshold(double linearDampingThreshold);
    void SetAngularDampingThreshold(double angularDampingThreshold);
    void SetMaxAngularSpeed(double maxAngularSpeed);

#ifdef EXPERIMENTAL
    void SetCylinderDragParameters(DragControl dragAxis, double dragFluidDensity, double dragCylinderMin, double dragCylinderMax, double dragCylinderRadius, double dragCylinderCoefficient);
    void SetDirectDragCoefficients(double linearDragCoefficientX, double linearDragCoefficientY, double linearDragCoefficientZ,
                                   double rotationalDragCoefficientX, double rotationalDragCoefficientY, double rotationalDragCoefficientZ);
#endif

    const double *GetPosition();
    const double *GetQuaternion();
    const double *GetLinearVelocity();
    const double *GetAngularVelocity();
    void GetPosition(pgd::Vector3 *pos);
    void GetQuaternion(pgd::Quaternion *quat);
    void GetRelativePosition(Body *rel, pgd::Vector3 *pos);
    void GetRelativeQuaternion(Body *rel, pgd::Quaternion *quat);
    void GetRelativeLinearVelocity(Body *rel, pgd::Vector3 *vel);
    void GetRelativeAngularVelocity(Body *rel, pgd::Vector3 *rVel);
    double GetMass() const;
    void GetMass(dMass *mass) const;
    double GetLinearKineticEnergy();
    void GetLinearKineticEnergy(dVector3 ke);
    double GetRotationalKineticEnergy();
    double GetGravitationalPotentialEnergy();
    dBodyID GetBodyID() const;

    void SetInitialPosition(double x, double y, double z);
    void SetInitialQuaternion(double n, double x, double y, double z);
    const double *GetInitialPosition();
    const double *GetInitialQuaternion();

    LimitTestResult TestLimits();
//    int SanityCheck(Body *otherBody, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    void EnterConstructionMode();
    void EnterRunMode();

#ifdef EXPERIMENTAL
    void ComputeDrag();
#endif

    // Utility
    static void ParallelAxis(dMass *massProperties, const double *translation, const double *quaternion, dMass *newMassProperties);
    static void ParallelAxis(double x, double y, double z, // transformation from centre of mass to new location (m)
                             double mass, // mass (kg)
                             double ixx, double iyy, double izz, double ixy, double iyz, double izx, // moments of inertia kgm2
                             double ang, // rotation angle (radians)
                             double ax, double ay, double az, // axis of rotation - must be unit length
                             double *ixxp, double *iyyp, double *izzp, double *ixyp, double *iyzp, double *izxp); // transformed moments of inertia about new coordinate system

    static double GetProjectedAngle(const pgd::Vector3 &planeNormal, const pgd::Vector3 &vector1, const pgd::Vector3 &vector2);
    static std::string MassCheck (const dMass *m);

    void SetGraphicFile1(const std::string &graphicFile) { m_graphicFile1 = graphicFile; }
    std::string GetGraphicFile1() const { return m_graphicFile1; }
    void SetGraphicFile2(const std::string &graphicFile) { m_graphicFile2 = graphicFile; }
    std::string GetGraphicFile2() const { return m_graphicFile2; }
    void SetGraphicFile3(const std::string &graphicFile) { m_graphicFile3 = graphicFile; }
    std::string GetGraphicFile3() const { return m_graphicFile3; }

    virtual std::string dumpToString() override;
    virtual std::string *createFromAttributes() override;
    virtual void saveToAttributes() override;
    virtual void appendToAttributes() override;

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
    dVector3 m_angularVelocityLowBound = {-DBL_MAX, -DBL_MAX, -DBL_MAX, 0};
    dVector3 m_angularVelocityHighBound = {DBL_MAX, DBL_MAX, DBL_MAX, 0};

    dVector3 m_initialPosition = {0, 0, 0, 0};
    dQuaternion m_initialQuaternion = {1, 0, 0, 0};

    // these are local copies that are really only used for bodies that are not attached to a simulation
    dVector3 m_currentPosition = {0, 0, 0, 0};
    dQuaternion m_currentQuaternion = {1, 0, 0, 0};

    std::string m_graphicFile1;
    std::string m_graphicFile2;
    std::string m_graphicFile3;

    // values used for saving loading only
    double m_LinearDamping = -1;
    double m_AngularDamping = -1;
    double m_LinearDampingThreshold = -1;
    double m_AngularDampingThreshold = -1;
    double m_MaxAngularSpeed = -1;

    bool m_constructionMode = false;


#ifdef EXPERIMENTAL
    DragControl m_dragControl = DragControl::NoDrag;
    double m_dragCoefficients[6] = {0, 0, 0, 0, 0, 0};
    double m_dragFluidDensity = 0;
    double m_dragCylinderMin = 0;
    double m_dragCylinderLength = 0;
    double m_dragCylinderRadius = 0;
    double m_dragCylinderCoefficient = 0;
#endif

};

#endif
