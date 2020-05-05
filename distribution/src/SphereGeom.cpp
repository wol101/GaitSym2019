/*
 *  SphereGeom.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 05/12/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "SphereGeom.h"
#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"


#include "ode/ode.h"

#include <string>

using namespace std::string_literals;

SphereGeom::SphereGeom(dSpaceID space, double radius)
{
    // create the geom
    setGeomID(dCreateSphere(space, radius));
    dGeomSetData(GetGeomID(), this);
}

std::string *SphereGeom::createFromAttributes()
{
    if (Geom::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (findAttribute("Radius"s, &buf) == nullptr) return lastErrorPtr();
    dGeomSphereSetRadius(GetGeomID(), GSUtil::Double(buf));
    return nullptr;
}

void SphereGeom::appendToAttributes()
{
    Geom::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Sphere"s);
    setAttribute("Radius"s, *GSUtil::ToString(dGeomSphereGetRadius(GetGeomID()), &buf));
    return;
}

double SphereGeom::radius() const
{
    return dGeomSphereGetRadius(GeomID());
}

void SphereGeom::setRadius(double radius)
{
    dGeomSphereSetRadius(GeomID(), radius);
}


