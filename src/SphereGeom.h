/*
 *  SphereGeom.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 05/12/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef SphereGeom_h
#define SphereGeom_h

#include "Geom.h"

class SphereGeom: public Geom
{
public:

    SphereGeom(dSpaceID space, double radius);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    double radius() const;
    void setRadius(double radius);
};


#endif

