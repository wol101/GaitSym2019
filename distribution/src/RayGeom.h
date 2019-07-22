/*
 *  SphereGeom.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 14/08/2009.
 *  Copyright 2009 Bill Sellers. All rights reserved.
 *
 */

#ifndef RAYGEOM_H
#define RAYGEOM_H

#include "Geom.h"

class RayGeom: public Geom
{
public:

    RayGeom(dSpaceID space, double length, double x, double y, double z, double dx, double dy, double dz);

    void SetParams(int firstContact, int backfaceCull, int closestHit);

};

#endif // RAYGEOM_H
