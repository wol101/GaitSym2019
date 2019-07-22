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
    void SetCylinderRadius(double radius) { m_CylinderRadius = radius; }
    void SetCylinderPosition(double x, double y, double z);
    void SetCylinderQuaternion(double q0, double q1, double q2, double q3);
    void SetCylinderAxis(double x, double y, double z);
    void SetCylinder(Marker *cylinderMarker);
    void SetNumWrapSegments(int num) { m_NumWrapSegments = num; }

    virtual void Calculate(double simulationTime);

    void GetOrigin(Body **body, dVector3 pos) { *body = m_OriginBody; pos[0] = m_OriginPosition.x; pos[1] = m_OriginPosition.y; pos[2] = m_OriginPosition.z; };
    void GetInsertion(Body **body, dVector3 pos) { *body = m_InsertionBody; pos[0] = m_InsertionPosition.x; pos[1] = m_InsertionPosition.y; pos[2] = m_InsertionPosition.z; };
    void GetCylinder(Body **body, dVector3 pos, double *radius, dQuaternion q);

    const pgd::Vector *GetPathCoordinates() { return m_PathCoordinates; }
    int GetNumPathCoordinates() { return m_NumPathCoordinates; }

    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::set<Marker *> *updateDependentMarkers();

    virtual std::string *CreateFromAttributes();
    virtual void AppendToAttributes();

private:

    int CylinderWrap(pgd::Vector &origin, pgd::Vector &insertion, double radius, int nWrapSegments, double maxAngle,
                     pgd::Vector &originForce, pgd::Vector &insertionForce, pgd::Vector &cylinderForce, pgd::Vector &cylinderForcePosition,
                     double *pathLength, pgd::Vector *pathCoordinates, int *numPathCoordinates);

    Body *m_OriginBody;
    pgd::Vector m_OriginPosition;
    Body *m_InsertionBody;
    pgd::Vector m_InsertionPosition;

    Body *m_CylinderBody;
    pgd::Vector m_CylinderPosition;
    pgd::Quaternion m_CylinderQuaternion;
    double m_CylinderRadius = 1;
    int m_NumWrapSegments = 2;

    int m_WrapStatus = -1;

    pgd::Vector *m_PathCoordinates = nullptr;
    int m_NumPathCoordinates = 0;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;
    Marker *m_cylinderMarker = nullptr;

};

#endif

