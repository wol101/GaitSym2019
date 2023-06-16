/*
 *  FacetedBox.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 31/05/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#ifndef FACETEDBOX_H
#define FACETEDBOX_H

#include "FacetedObject.h"
#include "PGDMath.h"
#include <vector>

class FacetedBox : public FacetedObject
{
public:
    // draw a box of dimensions lx, ly, lz with origin at the centre
    FacetedBox(double lx, double ly, double lz, const QColor &blendColour, double blendFraction);

private:
    void AddTriangleStrip(std::vector<pgd::Vector3> &triangleStrip);
    void AddTriangleFan(std::vector<pgd::Vector3> &triangleFan);

};

#endif
