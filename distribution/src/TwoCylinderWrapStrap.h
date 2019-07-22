/*
 *  TwoCylinderWrapStrap.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 19/12/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#ifndef TwoCylinderWrapStrap_h
#define TwoCylinderWrapStrap_h

#include "Strap.h"
#include "PGDMath.h"

class TwoCylinderWrapStrap: public Strap
{
public:

    TwoCylinderWrapStrap();
    virtual ~TwoCylinderWrapStrap();

    void SetOrigin(Body *body, dVector3 point);
    void SetInsertion(Body *body, dVector3 point);
    void SetOrigin(Marker *originMarker);
    void SetInsertion(Marker *insertionMarker);

    void SetCylinder1Body(Body *body);
    void SetCylinder1Radius(double radius) { m_Cylinder1Radius = radius; }
    void SetCylinder1Position(double x, double y, double z);
    void SetCylinderQuaternion(double q0, double q1, double q2, double q3);
    void SetCylinderAxis(double x, double y, double z);
    void SetCylinder1(Marker *cylinder1Marker);
    void SetCylinder2Body(Body *body);
    void SetCylinder2Radius(double radius) { m_Cylinder2Radius = radius; }
    void SetCylinder2Position(double x, double y, double z);
    void SetCylinder2(Marker *cylinder2Marker);
    void SetNumWrapSegments(int num) { m_NumWrapSegments = num; }

    virtual void Calculate(double simulationTime);

    void GetOrigin(Body **body, dVector3 pos) { *body = m_OriginBody; pos[0] = m_OriginPosition.x; pos[1] = m_OriginPosition.y; pos[2] = m_OriginPosition.z; }
    void GetInsertion(Body **body, dVector3 pos) { *body = m_InsertionBody; pos[0] = m_InsertionPosition.x; pos[1] = m_InsertionPosition.y; pos[2] = m_InsertionPosition.z; }
    void GetCylinder1(Body **body, dVector3 pos, double *radius, dQuaternion q);
    void GetCylinder2(Body **body, dVector3 pos, double *radius, dQuaternion q);

    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::set<Marker *> *updateDependentMarkers();

    virtual std::string *CreateFromAttributes();
    virtual void AppendToAttributes();

private:

    void TwoCylinderWrap(pgd::Vector &origin, pgd::Vector &insertion, pgd::Vector &cylinderPosition1, double radius1,
                         pgd::Vector &cylinderPosition2, double radius2, double tension, int nPointsPerCylinderArc, double maxAngle,
                         pgd::Vector &originForce, pgd::Vector &insertionForce, pgd::Vector &cylinderForce1, pgd::Vector &cylinderForcePosition1,
                         pgd::Vector &cylinderForce2, pgd::Vector &cylinderForcePosition2, double *pathLength,
                         pgd::Vector *pathCoordinates, int *numPathCoordinates, int *wrapOK);
    void FindCircleCircleTangents(pgd::Vector &c1, double radius1, pgd::Vector &c2, double radius2,
                                  pgd::Vector &outer1_p1, pgd::Vector &outer1_p2, pgd::Vector &outer2_p1, pgd::Vector &outer2_p2,
                                  pgd::Vector &inner1_p1, pgd::Vector &inner1_p2, pgd::Vector &inner2_p1, pgd::Vector &inner2_p2, int *number_of_tangents);
    void FindTangents(pgd::Vector &center, double radius, pgd::Vector &external_point, pgd::Vector &pt1, pgd::Vector &pt2, int *number_of_tangents);
    void FindCircleCircleIntersections(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1,
                                       pgd::Vector &intersection1, pgd::Vector &intersection2, int *number_of_intersections);

    double vector_distance2d(pgd::Vector &v1, pgd::Vector v2);
    double vector_distance3d(pgd::Vector &v1, pgd::Vector &v2);
    void vector_with_magnitude(pgd::Vector &v1, pgd::Vector &v2, double magnitude, pgd::Vector &v);

    Body *m_OriginBody = nullptr;
    pgd::Vector m_OriginPosition;
    Body *m_InsertionBody = nullptr;
    pgd::Vector m_InsertionPosition;

    Body *m_Cylinder1Body = nullptr;
    pgd::Vector m_Cylinder1Position;
    pgd::Quaternion m_CylinderQuaternion;
    double m_Cylinder1Radius = 1;
    Body *m_Cylinder2Body = nullptr;
    pgd::Vector m_Cylinder2Position;
    double m_Cylinder2Radius = 1;
    int m_NumWrapSegments = 2;

    int m_WrapStatus = -1;

    pgd::Vector *m_PathCoordinates = nullptr;
    int m_NumPathCoordinates = 0;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;
    Marker *m_cylinder1Marker = nullptr;
    Marker *m_cylinder2Marker = nullptr;
};

#endif

