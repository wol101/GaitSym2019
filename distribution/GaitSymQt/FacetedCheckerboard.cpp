/*
 *  FacetedCheckerboard.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 14/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "FacetedCheckerboard.h"
#include "PGDMath.h"

// draw a faceted checkerboard with origin at the centre
// uses 2 colours and the check resolution is cx, cy
// it draws 2x nx, ny checks

FacetedCheckerboard::FacetedCheckerboard(int nx, int ny, double cx, double cy,
        const QColor &colour1, const QColor &colour2, Qt3DCore::QNode *parent) : FacetedObject(parent)
{
    double x, y;
    double cx2 = cx / 2.0;
    double cy2 = cy / 2.0;
    this->AllocateMemory(size_t(nx * ny * 2 * 4));
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            x = +(ix * cx + cx2);
            y = +(iy * cy + cy2);
            if ((ix % 2 == 0 && iy % 2 == 0) || (ix % 2 == 1 && iy % 2 == 1)) this->SetColour(colour1);
            else this->SetColour(colour2);
            this->CentredRect(x, y, cx, cy);
            x = -(ix * cx + cx2);
            y = -(iy * cy + cy2);
            if ((ix % 2 == 0 && iy % 2 == 0) || (ix % 2 == 1 && iy % 2 == 1)) this->SetColour(colour1);
            else this->SetColour(colour2);
            this->CentredRect(x, y, cx, cy);
            x = -(ix * cx + cx2);
            y = +(iy * cy + cy2);
            if ((ix % 2 == 1 && iy % 2 == 0) || (ix % 2 == 0 && iy % 2 == 1)) this->SetColour(colour1);
            else this->SetColour(colour2);
            this->CentredRect(x, y, cx, cy);
            x = +(ix * cx + cx2);
            y = -(iy * cy + cy2);
            if ((ix % 2 == 1 && iy % 2 == 0) || (ix % 2 == 0 && iy % 2 == 1)) this->SetColour(colour1);
            else this->SetColour(colour2);
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

