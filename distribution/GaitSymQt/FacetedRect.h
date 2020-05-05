/*
 *  FacetedRect.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 31/05/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#ifndef FACETEDRECT_H
#define FACETEDRECT_H

#include "FacetedObject.h"

class FacetedRect : public FacetedObject
{
public:
    // draw a rect of dimensions lx, ly with origin at the centre
    FacetedRect(double lx, double ly, const QColor &blendColour, double blendFraction);

};

#endif // FACETEDRECT_H
