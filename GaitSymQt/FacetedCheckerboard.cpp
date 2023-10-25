/*
 *  FacetedCheckerboard.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 14/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "FacetedCheckerboard.h"

// draw a faceted checkerboard with origin at the centre
// uses 2 colours and the check resolution is cx, cy
// it draws 2x nx, ny checks

#ifdef USE_QT3D
FacetedCheckerboard::FacetedCheckerboard(size_t nx, size_t ny, double cx, double cy, const QColor &colour1, const QColor &colour2, Qt3DCore::QNode *parent) : FacetedObject(parent)
#else
FacetedCheckerboard::FacetedCheckerboard(size_t nx, size_t ny, double cx, double cy, const QColor &colour1, const QColor &colour2)
#endif
{
    double x, y;
    double cx2 = cx / 2.0;
    double cy2 = cy / 2.0;
    this->AllocateMemory(nx * ny * 2 * 4);
    for (size_t iy = 0; iy < ny; iy++)
    {
        for (size_t ix = 0; ix < nx; ix++)
        {
            x = +(ix * cx + cx2);
            y = +(iy * cy + cy2);
            if ((ix % 2 == 0 && iy % 2 == 0) || (ix % 2 == 1 && iy % 2 == 1)) this->setBlendColour(colour1, 0);
            else this->setBlendColour(colour2, 0);
            this->CentredRect(x, y, cx, cy);
            x = -(ix * cx + cx2);
            y = -(iy * cy + cy2);
            if ((ix % 2 == 0 && iy % 2 == 0) || (ix % 2 == 1 && iy % 2 == 1)) this->setBlendColour(colour1, 0);
            else this->setBlendColour(colour2, 0);
            this->CentredRect(x, y, cx, cy);
            x = -(ix * cx + cx2);
            y = +(iy * cy + cy2);
            if ((ix % 2 == 1 && iy % 2 == 0) || (ix % 2 == 0 && iy % 2 == 1)) this->setBlendColour(colour1, 0);
            else this->setBlendColour(colour2, 0);
            this->CentredRect(x, y, cx, cy);
            x = +(ix * cx + cx2);
            y = -(iy * cy + cy2);
            if ((ix % 2 == 1 && iy % 2 == 0) || (ix % 2 == 0 && iy % 2 == 1)) this->setBlendColour(colour1, 0);
            else this->setBlendColour(colour2, 0);
            this->CentredRect(x, y, cx, cy);
        }
    }
}

void FacetedCheckerboard::CentredRect(double x, double y, double xl, double yl)
{
    double vertices[12];
    double xl2 = xl / 2.0;
    double yl2 = yl / 2.0;
    vertices[0]  = x - xl2;
    vertices[1]  = y - yl2;
    vertices[2]  = 0.0;
    vertices[3]  = x + xl2;
    vertices[4]  = y - yl2;
    vertices[5]  = 0.0;
    vertices[6]  = x + xl2;
    vertices[7]  = y + yl2;
    vertices[8]  = 0.0;
    vertices[9]  = x - xl2;
    vertices[10] = y + yl2;
    vertices[11] = 0.0;
    AddPolygon(vertices, 4);
}

