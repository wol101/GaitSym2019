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

    pgd::Vector3 modelLocation() const;
    void setModelLocation(const pgd::Vector3 &modelLocation);

    pgd::Vector3 worldLocation() const;
    void setWorldLocation(const pgd::Vector3 &worldLocation);

    pgd::Vector3 screenLocation() const;
    void setScreenLocation(const pgd::Vector3 &screenLocation);

private:
    Drawable *m_drawable = nullptr;
    FacetedObject *m_facetedObject = nullptr;
    size_t m_triangleIndex = SIZE_MAX;
    pgd::Vector3 m_modelLocation;
    pgd::Vector3 m_worldLocation;
    pgd::Vector3 m_screenLocation;
};

#endif // INTERSECTIONHITS_H


