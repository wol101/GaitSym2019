/*
 *  CylinderWrapStrap.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef CylinderWrapStrap_h
#define CylinderWrapStrap_h

#include "Strap.h"
#include "PGDMath.h"

class Marker;

class CylinderWrapStrap: public Strap
{
public:

    CylinderWrapStrap();
    virtual ~CylinderWrapStrap();

//    void SetOrigin(Body *body, dVector3 point);
//    void SetInsertion(Body *body, dVector3 point);
    void SetOrigin(Marker *originMarker);
    void SetInsertion(Marker *insertionMarker);

//    void SetCylinderBody(Body *body);
    void SetCylinderRadius(double radius);
//    void SetCylinderPosition(double x, double y, double z);
//    void SetCylinderQuaternion(double q0, double q1, double q2, double q3);
//    void SetCylinderAxis(double x, double y, double z);
    void SetCylinder(Marker *cylinderMarker);
    void SetNumWrapSegments(int numWrapSegments);

    virtual void Calculate();

//    void GetOrigin(const Body **body, dVector3 pos) const;
//    void GetInsertion(const Body **body, dVector3 pos) const;
//    void GetCylinder(const Body **body, dVector3 pos, double *radius, dQuaternion q) const;

    Marker *GetOriginMarker() const;
    Marker *GetInsertionMarker() const;
    Marker *GetCylinderMarker() const;

    const std::vector<pgd::Vector3> *GetPathCoordinates();
    int GetNumWrapSegments();

//    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    double cylinderRadius() const;

private:

    int CylinderWrap(pgd::Vector3 &origin, pgd::Vector3 &insertion, double radius, int nWrapSegments, double maxAngle,
                     pgd::Vector3 &originForce, pgd::Vector3 &insertionForce, pgd::Vector3 &cylinderForce, pgd::Vector3 &cylinderForcePosition,
                     double *pathLength, std::vector<pgd::Vector3> *pathCoordinates);

//    Body *m_originBody;
//    pgd::Vector3 m_originPosition;
//    Body *m_insertionBody;
//    pgd::Vector3 m_insertionPosition;

//    Body *m_cylinderBody;
//    pgd::Vector3 m_cylinderPosition;
//    pgd::Quaternion m_cylinderQuaternion;
    double m_cylinderRadius = 1;
    int m_numWrapSegments = 0;

    int m_wrapStatus = -1;

    std::vector<pgd::Vector3> m_pathCoordinates;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;
    Marker *m_cylinderMarker = nullptr;

};

#endif

