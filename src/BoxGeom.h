/*
 *  BoxGeom.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 30/05/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#ifndef BOXGEOM_H
#define BOXGEOM_H

#include "Geom.h"

class BoxGeom : public Geom
{
public:
    BoxGeom(dSpaceID space, double lx, double ly, double lz);

    void GetDimensions(double *lx, double *ly, double *lz);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

};


#endif // BOXGEOM_H
