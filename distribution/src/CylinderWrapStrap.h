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

    void SetOrigin(Body *body, dVector3 point);
    void SetInsertion(Body *body, dVector3 point);
    void SetOrigin(Marker *originMarker);
    void SetInsertion(Marker *insertionMarker);

    void SetCylinderBody(Body *body);
    void SetCylinderRadius(double radius);
    void SetCylinderPosition(double x, double y, double z);
    void SetCylinderQuaternion(double q0, double q1, double q2, double q3);
    void SetCylinderAxis(double x, double y, double z);
    void SetCylinder(Marker *cylinderMarker);
    void SetNumWrapSegments(int num);

    virtual void Calculate();

    void GetOrigin(const Body **body, dVector3 pos) const;
    void GetInsertion(const Body **body, dVector3 pos) const;
    void GetCylinder(const Body **body, dVector3 pos, double *radius, dQuaternion q) const;

    const pgd::Vector *GetPathCoordinates() { return m_pathCoordinates.data(); }
    int GetNumPathCoordinates() { return m_numPathCoordinates; }

    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::set<Marker *> *updateDependentMarkers();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    int CylinderWrap(pgd::Vector &origin, pgd::Vector &insertion, double radius, int nWrapSegments, double maxAngle,
                     pgd::Vector &originForce, pgd::Vector &insertionForce, pgd::Vector &cylinderForce, pgd::Vector &cylinderForcePosition,
                     double *pathLength, pgd::Vector *pathCoordinates, int *numPathCoordinates);

    Body *m_originBody;
    pgd::Vector m_originPosition;
    Body *m_insertionBody;
    pgd::Vector m_insertionPosition;

    Body *m_cylinderBody;
    pgd::Vector m_cylinderPosition;
    pgd::Quaternion m_cylinderQuaternion;
    double m_cylinderRadius = 1;
    int m_numWrapSegments = 2;

    int m_wrapStatus = -1;

    std::vector<pgd::Vector> m_pathCoordinates;
    int m_numPathCoordinates = 0;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;
    Marker *m_cylinderMarker = nullptr;

};

#endif

