/*
 *  TrimeshGeom.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 11/08/2009.
 *  Copyright 2009 Bill Sellers. All rights reserved.
 *
 */

#ifndef TRIMESHGEOM_H
#define TRIMESHGEOM_H

#include "Geom.h"

class StridedVertex;
class StridedTri;
class GimpactStridedVertex;

class TrimeshGeom : public Geom
{
public:
    TrimeshGeom(dSpaceID space, const std::vector<double> &vertexList);
    virtual ~TrimeshGeom();

private:

#ifdef USE_GIMPACT
    GimpactStridedVertex *m_Vertices = nullptr;
#else
    double *m_Vertices = nullptr;
#endif
    int m_NumVertices = 0;
    int m_VertexStride = 0;
    dTriIndex *m_TriIndexes = nullptr;
    int m_NumTriIndexes = 0;
    int m_TriStride = 0;
    dTriMeshDataID m_TriMeshDataID = nullptr;

};

#endif // TRIMESHGEOM_H

