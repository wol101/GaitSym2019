/*
 *  TrimeshGeom.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 11/08/2009.
 *  Copyright 2009 Bill Sellers. All rights reserved.
 *
 */


#include "ode/ode.h"
#include <string>

#include "TrimeshGeom.h"

// create the trimesh object
// m_vertexList is a list of traingular vertices in FacetedObject format
// i.e x1y1z1x2y2z2x3y3z3 for each triangle lacked as a single list
TrimeshGeom::TrimeshGeom(dSpaceID space, const std::vector<double> &vertexList)
{
    size_t i;
    m_VertexStride = 3 * sizeof(double);
    m_TriStride = 3 * sizeof(dTriIndex);

    m_NumVertices = int((vertexList.size() / 3));
    m_NumTriIndexes = int((vertexList.size() / 3));

    m_Vertices = new double[(vertexList.size() / 3) * 3];
    m_TriIndexes = new dTriIndex[(vertexList.size() / 3)];

    for (i = 0; i < (vertexList.size() / 3); i++)
    {
        m_Vertices[i * 3] = vertexList[i * 3];
        m_Vertices[i * 3 + 1] = vertexList[i * 3 + 1];
        m_Vertices[i * 3 + 2] = vertexList[i * 3 + 2];
    }

    for (i = 0; i < (vertexList.size() / 3); i++)
    {
        m_TriIndexes[i] = dTriIndex(i);
    }

    m_TriMeshDataID = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildDouble (m_TriMeshDataID, m_Vertices, m_VertexStride, m_NumVertices, m_TriIndexes, m_NumTriIndexes, m_TriStride);

    setGeomID(dCreateTriMesh (space, m_TriMeshDataID, nullptr, nullptr, nullptr));
    dGeomSetData(GetGeomID(), this);
}

TrimeshGeom::~TrimeshGeom()
{
    dGeomTriMeshDataDestroy(m_TriMeshDataID);
    delete [] m_Vertices;
    delete [] m_TriIndexes;
}


