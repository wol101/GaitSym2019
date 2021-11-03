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

//    void SetOrigin(Body *body, const dVector3 point);
//    void SetInsertion(Body *body, const dVector3 point);
    void SetOrigin(Marker *originMarker);
    void SetInsertion(Marker *insertionMarker);

//    void SetCylinder1Body(Body *body);
    void SetCylinder1Radius(double radius);
//    void SetCylinder1Position(double x, double y, double z);
//    void SetCylinderQuaternion(double q0, double q1, double q2, double q3);
//    void SetCylinderAxis(double x, double y, double z);
    void SetCylinder1(Marker *cylinder1Marker);
//    void SetCylinder2Body(Body *body);
    void SetCylinder2Radius(double radius);
//    void SetCylinder2Position(double x, double y, double z);
    void SetCylinder2(Marker *cylinder2Marker);
    void SetNumWrapSegments(int numWrapSegments);

    virtual void Calculate();

//    void GetOrigin(const Body **body, dVector3 pos) const;
//    void GetInsertion(const Body **body, dVector3 pos) const;
//    void GetCylinder1(const Body **body, dVector3 pos, double *radius, dQuaternion q) const;
//    void GetCylinder2(const Body **body, dVector3 pos, double *radius, dQuaternion q) const;

    Marker *GetOriginMarker() const;
    Marker *GetInsertionMarker() const;
    Marker *GetCylinder1Marker() const;
    Marker *GetCylinder2Marker() const;

    const std::vector<pgd::Vector3> *GetPathCoordinates();
    int GetNumWrapSegments();

//    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    double Cylinder1Radius() const;
    double Cylinder2Radius() const;

private:

    void TwoCylinderWrap(pgd::Vector3 &origin, pgd::Vector3 &insertion, pgd::Vector3 &cylinderPosition1, double radius1,
                         pgd::Vector3 &cylinderPosition2, double radius2, double tension, int nPointsPerCylinderArc, double maxAngle,
                         pgd::Vector3 &originForce, pgd::Vector3 &insertionForce, pgd::Vector3 &cylinderForce1, pgd::Vector3 &cylinderForcePosition1,
                         pgd::Vector3 &cylinderForce2, pgd::Vector3 &cylinderForcePosition2, double *pathLength,
                         std::vector<pgd::Vector3> *pathCoordinates, int *wrapOK);
    void FindCircleCircleTangents(pgd::Vector3 &c1, double radius1, pgd::Vector3 &c2, double radius2,
                                  pgd::Vector3 &outer1_p1, pgd::Vector3 &outer1_p2, pgd::Vector3 &outer2_p1, pgd::Vector3 &outer2_p2,
                                  pgd::Vector3 &inner1_p1, pgd::Vector3 &inner1_p2, pgd::Vector3 &inner2_p1, pgd::Vector3 &inner2_p2, int *number_of_tangents);
    void FindTangents(pgd::Vector3 &center, double radius, pgd::Vector3 &external_point, pgd::Vector3 &pt1, pgd::Vector3 &pt2, int *number_of_tangents);
    void FindCircleCircleIntersections(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1,
                                       pgd::Vector3 &intersection1, pgd::Vector3 &intersection2, int *number_of_intersections);

    double vector_distance2d(pgd::Vector3 &v1, pgd::Vector3 v2);
    double vector_distance3d(pgd::Vector3 &v1, pgd::Vector3 &v2);
    void vector_with_magnitude(pgd::Vector3 &v1, pgd::Vector3 &v2, double magnitude, pgd::Vector3 &v);

//    Body *m_originBody = nullptr;
//    pgd::Vector3 m_originPosition;
//    Body *m_insertionBody = nullptr;
//    pgd::Vector3 m_insertionPosition;

//    Body *m_cylinder1Body = nullptr;
//    pgd::Vector3 m_cylinder1Position;
//    pgd::Quaternion m_cylinderQuaternion;
    double m_cylinder1Radius = 1;
//    Body *m_cylinder2Body = nullptr;
//    pgd::Vector3 m_cylinder2Position;
    double m_cylinder2Radius = 1;
    int m_numWrapSegments = 0;

    int m_wrapStatus = -1;

    std::vector<pgd::Vector3> m_pathCoordinates;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;
    Marker *m_cylinder1Marker = nullptr;
    Marker *m_cylinder2Marker = nullptr;
};

#endif

