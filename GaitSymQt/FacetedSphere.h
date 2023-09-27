/*
 *  FacetedSphere.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 03/01/2006.
 *  Copyright 2006 Bill Sellers. All rights reserved.
 *
 */

#ifndef FacetedSphere_h
#define FacetedSphere_h

#include <sstream>

#include "FacetedObject.h"

class FacetedSphere: public FacetedObject
{
public:
#ifdef USE_QT3D
    FacetedSphere(double radius, size_t maxlevels, const QColor &blendColour, double blendFraction, Qt3DCore::QNode *parent = nullptr);
#else
    FacetedSphere(double radius, size_t maxlevels, const QColor &blendColour, double blendFraction);
#endif

    static size_t EstimateLevel(size_t requestedFaces, size_t *actualFaces = nullptr);

    virtual void WritePOVRay(std::ostringstream &theString);

private:
    size_t m_Level;
    double m_Radius;
};

#endif
