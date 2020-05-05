/*
 *  IntersectionHits.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 12/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef INTERSECTIONHITS_H
#define INTERSECTIONHITS_H

#include "PGDMath.h"

#include <limits.h>

class Drawable;
class FacetedObject;

class IntersectionHits
{
public:
    IntersectionHits();

    Drawable *drawable() const;
    void setDrawable(Drawable *drawable);

    FacetedObject *facetedObject() const;
    void setFacetedObject(FacetedObject *facetedObject);

    size_t triangleIndex() const;
    void setTriangleIndex(const size_t &triangleIndex);

    pgd::Vector modelLocation() const;
    void setModelLocation(const pgd::Vector &modelLocation);

    pgd::Vector worldLocation() const;
    void setWorldLocation(const pgd::Vector &worldLocation);

    pgd::Vector screenLocation() const;
    void setScreenLocation(const pgd::Vector &screenLocation);

private:
    Drawable *m_drawable = nullptr;
    FacetedObject *m_facetedObject = nullptr;
    size_t m_triangleIndex = SIZE_MAX;
    pgd::Vector m_modelLocation;
    pgd::Vector m_worldLocation;
    pgd::Vector m_screenLocation;
};

#endif // INTERSECTIONHITS_H


