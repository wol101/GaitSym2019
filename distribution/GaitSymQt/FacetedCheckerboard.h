/*
 *  FacetedCheckerboard.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 14/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef FACETEDCHECKERBOARD_H
#define FACETEDCHECKERBOARD_H

#include "FacetedObject.h"
#include "PGDMath.h"
#include <vector>

class FacetedCheckerboard : public FacetedObject
{
public:
    // draw a rect of dimensions lx, ly with origin at the centre
    FacetedCheckerboard(int nx, int ny, double cx, double cy, const QColor &colour1,
                        const QColor &colour2, Qt3DCore::QNode *parent = nullptr);

private:
    void CentredRect(double x, double y, double xl, double yl);

};

#endif // FACETEDCHECKERBOARD_H
