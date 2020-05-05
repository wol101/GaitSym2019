/*
 *  FacetedConicSegment.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 06/01/2006.
 *  Copyright 2006 Bill Sellers. All rights reserved.
 *
 */

#ifndef FacetedConicSegment_h
#define FacetedConicSegment_h

#include <sstream>
#include "FacetedObject.h"

class FacetedConicSegment: public FacetedObject
{
public:
    FacetedConicSegment(double length, double r1, double r2, size_t sides, double ox, double oy, double oz, const QColor &blendColour, double blendFraction);

    virtual void WritePOVRay(std::ostringstream &theString);

private:
    double m_R1;
    double m_R2;
    double m_Length;
    double m_OX;
    double m_OY;
    double m_OZ;
    size_t m_Sides;
};

#endif
