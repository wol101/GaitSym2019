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

    void SetOrigin(Body *body, const dVector3 point);
    void SetInsertion(Body *body, const dVector3 point);
    void SetOrigin(Marker *originMarker);
    void SetInsertion(Marker *insertionMarker);

    void SetCylinder1Body(Body *body);
    void SetCylinder1Radius(double radius);
    void SetCylinder1Position(double x, double y, double z);
    void SetCylinderQuaternion(double q0, double q1, double q2, double q3);
    void SetCylinderAxis(double x, double y, double z);
    void SetCylinder1(Marker *cylinder1Marker);
    void SetCylinder2Body(Body *body);
    void SetCylinder2Radius(double radius);
    void SetCylinder2Position(double x, double y, double z);
    void SetCylinder2(Marker *cylinder2Marker);
    void SetNumWrapSegments(int num);

    virtual void Calculate();

    void GetOrigin(const Body **body, dVector3 pos) const;
    void GetInsertion(const Body **body, dVector3 pos) const;
    void GetCylinder1(const Body **body, dVector3 pos, double *radius, dQuaternion q) const;
    void GetCylinder2(const Body **body, dVector3 pos, double *radius, dQuaternion q) const;

    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::set<Marker *> *updateDependentMarkers();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

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

    std::vector<pgd::Vector> m_PathCoordinates;
    int m_NumPathCoordinates = 0;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;
    Marker *m_cylinder1Marker = nullptr;
    Marker *m_cylinder2Marker = nullptr;
};

#endif

