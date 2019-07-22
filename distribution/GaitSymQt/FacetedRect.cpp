/*
 *  FacetedRect.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 31/05/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#include "FacetedRect.h"
#include "PGDMath.h"

// draw a rect of dimensions lx, ly with origin at the centre

FacetedRect::FacetedRect(double lx, double ly, const QColor &colour,
                         Qt3DCore::QNode *parent) : FacetedObject(parent)
{
    SetColour(colour);

    lx *= 0.5;
    ly *= 0.5;
    size_t nSides = 4;
    double vertices[12];
    AllocateMemory(2);

    vertices[0] = -lx;
    vertices[1] = -ly;
    vertices[2] = 0;
    vertices[3] = +lx;
    vertices[4] = -ly;
    vertices[5] = 0;
    vertices[6] = +lx;
    vertices[7] = +ly;
    vertices[8] = 0;
    vertices[9] = -lx;
    vertices[10] = +ly;
    vertices[11] = 0;

    AddPolygon(vertices, nSides);

//    qDebug() << "FacetedRect " << GetNumTriangles() << " triangles created\n";
}


