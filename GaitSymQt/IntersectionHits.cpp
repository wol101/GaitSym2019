/*
 *  IntersectionHits.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 12/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "IntersectionHits.h"

IntersectionHits::IntersectionHits()
{

}

Drawable *IntersectionHits::drawable() const
{
    return m_drawable;
}

void IntersectionHits::setDrawable(Drawable *drawable)
{
    m_drawable = drawable;
}

FacetedObject *IntersectionHits::facetedObject() const
{
    return m_facetedObject;
}

void IntersectionHits::setFacetedObject(FacetedObject *facetedObject)
{
    m_facetedObject = facetedObject;
}

size_t IntersectionHits::triangleIndex() const
{
    return m_triangleIndex;
}

void IntersectionHits::setTriangleIndex(const size_t &triangleIndex)
{
    m_triangleIndex = triangleIndex;
}

pgd::Vector3 IntersectionHits::modelLocation() const
{
    return m_modelLocation;
}

void IntersectionHits::setModelLocation(const pgd::Vector3 &modelLocation)
{
    m_modelLocation = modelLocation;
}

pgd::Vector3 IntersectionHits::worldLocation() const
{
    return m_worldLocation;
}

void IntersectionHits::setWorldLocation(const pgd::Vector3 &worldLocation)
{
    m_worldLocation = worldLocation;
}

pgd::Vector3 IntersectionHits::screenLocation() const
{
    return m_screenLocation;
}

void IntersectionHits::setScreenLocation(const pgd::Vector3 &screenLocation)
{
    m_screenLocation = screenLocation;
}


