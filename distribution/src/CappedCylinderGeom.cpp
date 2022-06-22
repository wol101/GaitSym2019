/*
 *  CappedCylinderGeom.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 28/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "CappedCylinderGeom.h"
#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"


#include "ode/ode.h"

#include <string>

using namespace std::string_literals;

CappedCylinderGeom::CappedCylinderGeom(dSpaceID space, double radius, double length)
{
    // create the geom
    setGeomID(dCreateCapsule(space, radius, length));

    dGeomSetData(GetGeomID(), this);
}

std::string *CappedCylinderGeom::createFromAttributes()
{
    if (Geom::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (findAttribute("Radius"s, &buf) == nullptr) return lastErrorPtr();
    double radius = GSUtil::Double(buf);
    if (findAttribute("Length"s, &buf) == nullptr) return lastErrorPtr();
    double length = GSUtil::Double(buf);
    dGeomCapsuleSetParams(GetGeomID(), radius, length);

    return nullptr;
}

void CappedCylinderGeom::appendToAttributes()
{
    Geom::appendToAttributes();
    std::string buf;

    setAttribute("Type"s, "CappedCylinder"s);
    double radius, length;
    dGeomCapsuleGetParams(GetGeomID(), &radius, &length);
    setAttribute("Radius"s, *GSUtil::ToString(radius, &buf));
    setAttribute("Length"s, *GSUtil::ToString(length, &buf));
    return;
}

void CappedCylinderGeom::setLengthRadius(double length, double radius)
{
    dGeomCapsuleSetParams(GetGeomID(), radius, length);
}

void CappedCylinderGeom::getLengthRadius(double *length, double *radius) const
{
    dGeomCapsuleGetParams(GeomID(), radius, length);
}



