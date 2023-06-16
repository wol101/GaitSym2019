/*
 *  RayGeom.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 14/08/2009.
 *  Copyright 2009 Bill Sellers. All rights reserved.
 *
 */

#include "ode/ode.h"
#include <string>

#include "RayGeom.h"

// creates the ray geometry with defined
// length
// origin x, y, z
// direction dx, dy, dz
RayGeom::RayGeom(dSpaceID space, double length, double x, double y, double z, double dx, double dy, double dz)
{
    // create the geom
    setGeomID(dCreateRay(space, length));
    dGeomRaySet(GetGeomID(), x, y, z, dx, dy, dz);
    dGeomSetData(GetGeomID(), this);
}

// set some ray control parameters
void RayGeom::SetParams(int firstContact, int backfaceCull, int closestHit)
{
#ifdef USE_OLD_ODE
    dGeomRaySetParams(GetGeomID(), firstContact, backfaceCull);
#else
    dGeomRaySetFirstContact (GetGeomID(), firstContact);
    dGeomRaySetBackfaceCull (GetGeomID(), backfaceCull);
#endif
    dGeomRaySetClosestHit (GetGeomID(), closestHit);
}

