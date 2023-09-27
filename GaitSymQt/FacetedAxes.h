/*
 *  FacetedAxes.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 13/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef FACETEDAXES_H
#define FACETEDAXES_H

#include "FacetedObject.h"

class FacetedAxes : public FacetedObject
{
public:
#ifdef USE_QT3D
    FacetedAxes(Qt3DCore::QNode *parent = nullptr);
#else
    FacetedAxes();
#endif
};

#endif // FACETEDAXES_H
