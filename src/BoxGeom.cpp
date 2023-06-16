/*
 *  BoxGeom.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 30/05/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#include "BoxGeom.h"
#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"


#include "ode/ode.h"

#include <string>

using namespace std::string_literals;

BoxGeom::BoxGeom(dSpaceID space, double lx, double ly, double lz)
{
    // create the geom
    setGeomID(dCreateBox(space, lx, ly, lz));
    dGeomSetData(GetGeomID(), this);
}

void BoxGeom::GetDimensions(double *lx, double *ly, double *lz)
{
    dVector3 result;
    dGeomBoxGetLengths(GetGeomID(), result);
    *lx = result[0];
    *ly = result[1];
    *lz = result[2];
}

std::string *BoxGeom::createFromAttributes()
{
    if (Geom::createFromAttributes()) return lastErrorPtr();
    std::string buf;

    if (findAttribute("LengthX"s, &buf) == nullptr) return lastErrorPtr();
    double lengthX = GSUtil::Double(buf);
    if (findAttribute("LengthY"s, &buf) == nullptr) return lastErrorPtr();
    double lengthY = GSUtil::Double(buf);
    if (findAttribute("LengthZ"s, &buf) == nullptr) return lastErrorPtr();
    double lengthZ = GSUtil::Double(buf);
    dGeomBoxSetLengths(GetGeomID(), lengthX, lengthY, lengthZ);

    return nullptr;
}

void BoxGeom::appendToAttributes()
{
    Geom::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Box"s);
    dVector3 result;
    dGeomBoxGetLengths(GetGeomID(), result);
    setAttribute("LengthX"s, *GSUtil::ToString(result[0], &buf));
    setAttribute("LengthY"s, *GSUtil::ToString(result[1], &buf));
    setAttribute("LengthZ"s, *GSUtil::ToString(result[2], &buf));

    return;
}

