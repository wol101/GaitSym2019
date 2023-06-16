/*
 *  PlaneGeom.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 13/09/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef PlaneGeom_h
#define PlaneGeom_h

#include "Geom.h"

#include "ode/ode.h"

class PlaneGeom: public Geom
{
public:

    PlaneGeom(dSpaceID space, double a, double b, double c, double d);
    virtual ~PlaneGeom();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

};


#endif
