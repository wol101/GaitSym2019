/*
 *  FacetedCappedCylinder.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 06/02/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#ifndef FACETEDCAPPEDCYLINDER_H
#define FACETEDCAPPEDCYLINDER_H

#include "FacetedObject.h"
#include <vector>

class FacetedCappedCylinder : public FacetedObject
{
public:
    // draw a capped cylinder of length l and radius r, aligned along the x axis
#ifdef USE_QT3D
    FacetedCappedCylinder(double l, double r, size_t capped_cylinder_quality, const QColor &blendColour, double blendFraction, Qt3DCore::QNode *parent = nullptr);
#else
    FacetedCappedCylinder(double l, double r, size_t capped_cylinder_quality, const QColor &blendColour, double blendFraction);
#endif

private:
    void AddTriangleStrip(std::vector<pgd::Vector3> &triangleStrip);

};


#endif // FACETEDCAPPEDCYLINDER_H
