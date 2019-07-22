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
    FacetedConicSegment(double length, double r1, double r2, int sides, double ox, double oy, double oz,
                        const QColor &colour, Qt3DCore::QNode *parent);

    virtual void WritePOVRay(std::ostringstream &theString);

private:
    double m_R1;
    double m_R2;
    double m_Length;
    double m_OX;
    double m_OY;
    double m_OZ;
    int m_Sides;
};

#endif
