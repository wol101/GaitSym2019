/*
 *  FacetedRect.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 31/05/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#include "FacetedRect.h"

// draw a rect of dimensions lx, ly with origin at the centre
// creates suitable uv coordinates too

#ifdef USE_QT3D
FacetedRect::FacetedRect(double lx, double ly, const QColor &blendColour, double blendFraction, Qt3DCore::QNode *parent) : FacetedObject(parent)
#else
FacetedRect::FacetedRect(double lx, double ly, const QColor &blendColour, double blendFraction)
#endif
{
    setBlendColour(blendColour, blendFraction);

    lx *= 0.5;
    ly *= 0.5;
    size_t nSides = 4;
    double vertices[12];
    double uvs[8];
    AllocateMemory(2);

    vertices[0] = -lx;
    vertices[1] = -ly;
    vertices[2] = 0;
    uvs[0] = 0;
    uvs[1] = 0,
    vertices[3] = +lx;
    vertices[4] = -ly;
    vertices[5] = 0;
    uvs[2] = 1;
    uvs[3] = 0,
    vertices[6] = +lx;
    vertices[7] = +ly;
    vertices[8] = 0;
    uvs[4] = 1;
    uvs[5] = 1,
    vertices[9] = -lx;
    vertices[10] = +ly;
    vertices[11] = 0;
    uvs[6] = 0;
    uvs[7] = 1,

    AddPolygon(vertices, nSides, nullptr, uvs);

//    qDebug() << "FacetedRect " << GetNumTriangles() << " triangles created\n";
}


