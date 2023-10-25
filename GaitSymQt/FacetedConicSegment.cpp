/*
 *  FacetedConicSegment.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 06/01/2006.
 *  Copyright 2006 Bill Sellers. All rights reserved.
 *
 */


#include "FacetedConicSegment.h"
#include "GSUtil.h"

#include <sstream>
#include <iomanip>
#include <cmath>

// create a conic segment structure with axis along z axis
// radii are specified by r1 and r2 and the origin is the centre of r1
// if r2 == 0 then draw a cone
// if r1 == r2 then draw a cylinder
#ifdef USE_QT3D
FacetedConicSegment::FacetedConicSegment(double l, double r1, double r2, size_t sides, double ox, double oy, double oz, const QColor &blendColour, double blendFraction, Qt3DCore::QNode *parent) : FacetedObject(parent)
#else
FacetedConicSegment::FacetedConicSegment(double l, double r1, double r2, size_t sides, double ox, double oy, double oz, const QColor &blendColour, double blendFraction)
#endif
{
    setBlendColour(blendColour, blendFraction);
    m_R1 = r1;
    m_R2 = r2;
    m_Length = l;
    m_OX = ox;
    m_OY = oy;
    m_OZ = oz;
    m_Sides = sides;

    AllocateMemory(sides * 4);

    size_t i;
    double theta = 2 * M_PI / sides;
    pgd::Vector3 vertex;
    std::vector<pgd::Vector3> vertexList;
    double triangle[3 * 3];

    vertex.x = ox;
    vertex.y = oy;
    vertex.z = oz;
    vertexList.push_back(vertex);
    vertex.z = l + oz;
    vertexList.push_back(vertex);

    if (r2 != 0)
    {
        vertex.z = oz;
        for (i = 0; i < sides; i++)
        {
            vertex.x = r1 * cos(theta * i) + ox;
            vertex.y = r1 * sin(theta * i) + oy;
            vertexList.push_back(vertex);
        }
        vertex.z = l + oz;
        if (r1 == r2)
        {
            for (i = 0; i < sides; i++)
            {
                vertex.x = vertexList[i + 2].x;
                vertex.y = vertexList[i + 2].y;
                vertexList.push_back(vertex);
            }
        }
        else
        {
            for (i = 0; i < sides; i++)
            {
                vertex.x = r2 * cos(theta * i) + ox;
                vertex.y = r2 * sin(theta * i) + oy;
                vertexList.push_back(vertex);
            }
        }

        // first end cap
        for (i = 0; i < sides; i++)
        {
            if (i < sides - 1)
            {
                triangle[0] = vertexList[0].x;
                triangle[1] = vertexList[0].y;
                triangle[2] = vertexList[0].z;
                triangle[3] = vertexList[i + 2].x;
                triangle[4] = vertexList[i + 2].y;
                triangle[5] = vertexList[i + 2].z;
                triangle[6] = vertexList[i + 3].x;
                triangle[7] = vertexList[i + 3].y;
                triangle[8] = vertexList[i + 3].z;
                //face->SetVertex(0, 0);
                //face->SetVertex(1, i + 2);
                //face->SetVertex(2, i + 3);
            }
            else
            {
                triangle[0] = vertexList[0].x;
                triangle[1] = vertexList[0].y;
                triangle[2] = vertexList[0].z;
                triangle[3] = vertexList[i + 2].x;
                triangle[4] = vertexList[i + 2].y;
                triangle[5] = vertexList[i + 2].z;
                triangle[6] = vertexList[2].x;
                triangle[7] = vertexList[2].y;
                triangle[8] = vertexList[2].z;
                //face->SetVertex(0, 0);
                //face->SetVertex(1, i + 2);
                //face->SetVertex(2, 2);
            }
            AddTriangle(triangle);
        }

        // sides
        for (i = 0; i < sides; i++)
        {
            if (i < sides - 1)
            {
                triangle[0] = vertexList[i + 2].x;
                triangle[1] = vertexList[i + 2].y;
                triangle[2] = vertexList[i + 2].z;
                triangle[3] = vertexList[i + 2 + sides].x;
                triangle[4] = vertexList[i + 2 + sides].y;
                triangle[5] = vertexList[i + 2 + sides].z;
                triangle[6] = vertexList[i + 3 + sides].x;
                triangle[7] = vertexList[i + 3 + sides].y;
                triangle[8] = vertexList[i + 3 + sides].z;
                //face->SetVertex(0, i + 2);
                //face->SetVertex(1, i + 2 + sides);
                //face->SetVertex(2, i + 3 + sides);
                AddTriangle(triangle);
                triangle[0] = vertexList[i + 3 + sides].x;
                triangle[1] = vertexList[i + 3 + sides].y;
                triangle[2] = vertexList[i + 3 + sides].z;
                triangle[3] = vertexList[i + 3].x;
                triangle[4] = vertexList[i + 3].y;
                triangle[5] = vertexList[i + 3].z;
                triangle[6] = vertexList[i + 2].x;
                triangle[7] = vertexList[i + 2].y;
                triangle[8] = vertexList[i + 2].z;
                //face->SetVertex(0, i + 3 + sides);
                //face->SetVertex(1, i + 3);
                //face->SetVertex(2, i + 2);
                AddTriangle(triangle);
            }
            else
            {
                triangle[0] = vertexList[i + 2].x;
                triangle[1] = vertexList[i + 2].y;
                triangle[2] = vertexList[i + 2].z;
                triangle[3] = vertexList[i + 2 + sides].x;
                triangle[4] = vertexList[i + 2 + sides].y;
                triangle[5] = vertexList[i + 2 + sides].z;
                triangle[6] = vertexList[2 + sides].x;
                triangle[7] = vertexList[2 + sides].y;
                triangle[8] = vertexList[2 + sides].z;
                //face->SetVertex(0, i + 2);
                //face->SetVertex(1, i + 2 + sides);
                //face->SetVertex(2, 2 + sides);
                AddTriangle(triangle);
                triangle[0] = vertexList[2 + sides].x;
                triangle[1] = vertexList[2 + sides].y;
                triangle[2] = vertexList[2 + sides].z;
                triangle[3] = vertexList[2].x;
                triangle[4] = vertexList[2].y;
                triangle[5] = vertexList[2].z;
                triangle[6] = vertexList[i + 2].x;
                triangle[7] = vertexList[i + 2].y;
                triangle[8] = vertexList[i + 2].z;
                //face->SetVertex(0, 2 + sides);
                //face->SetVertex(1, 2);
                //face->SetVertex(2, i + 2);
                AddTriangle(triangle);
            }
        }

        // final end cap
        for (i = 0; i < sides; i++)
        {
            if (i < sides - 1)
            {
                triangle[0] = vertexList[1].x;
                triangle[1] = vertexList[1].y;
                triangle[2] = vertexList[1].z;
                triangle[3] = vertexList[i + 3 + sides].x;
                triangle[4] = vertexList[i + 3 + sides].y;
                triangle[5] = vertexList[i + 3 + sides].z;
                triangle[6] = vertexList[i + 2 + sides].x;
                triangle[7] = vertexList[i + 2 + sides].y;
                triangle[8] = vertexList[i + 2 + sides].z;
                //face->SetVertex(0, 1);
                //face->SetVertex(1, i + 3 + sides);
                //face->SetVertex(2, i + 2 + sides);
            }
            else
            {
                triangle[0] = vertexList[1].x;
                triangle[1] = vertexList[1].y;
                triangle[2] = vertexList[1].z;
                triangle[3] = vertexList[2 + sides].x;
                triangle[4] = vertexList[2 + sides].y;
                triangle[5] = vertexList[2 + sides].z;
                triangle[6] = vertexList[i + 2 + sides].x;
                triangle[7] = vertexList[i + 2 + sides].y;
                triangle[8] = vertexList[i + 2 + sides].z;
                //face->SetVertex(0, 1);
                //face->SetVertex(1, 2 + sides);
                //face->SetVertex(2, i + 2 + sides);
            }
            AddTriangle(triangle);
        }

    }
    else
    {
        // draw a cone with base radius r1

        // base vertices
        vertex.z = oz;
        for (i = 0; i < sides; i++)
        {
            vertex.x = r1 * cos(theta * i) + ox;
            vertex.y = r1 * sin(theta * i) + oy;
            vertexList.push_back(vertex);
        }

        // base
        for (i = 0; i < sides; i++)
        {
            if (i < sides - 1)
            {
                triangle[0] = vertexList[0].x;
                triangle[1] = vertexList[0].y;
                triangle[2] = vertexList[0].z;
                triangle[3] = vertexList[i + 2].x;
                triangle[4] = vertexList[i + 2].y;
                triangle[5] = vertexList[i + 2].z;
                triangle[6] = vertexList[i + 3].x;
                triangle[7] = vertexList[i + 3].y;
                triangle[8] = vertexList[i + 3].z;
                //face->SetVertex(0, 0);
                //face->SetVertex(1, i + 2);
                //face->SetVertex(2, i + 3);
            }
            else
            {
                triangle[0] = vertexList[0].x;
                triangle[1] = vertexList[0].y;
                triangle[2] = vertexList[0].z;
                triangle[3] = vertexList[i + 2].x;
                triangle[4] = vertexList[i + 2].y;
                triangle[5] = vertexList[i + 2].z;
                triangle[6] = vertexList[2].x;
                triangle[7] = vertexList[2].y;
                triangle[8] = vertexList[2].z;
                //face->SetVertex(0, 0);
                //face->SetVertex(1, i + 2);
                //face->SetVertex(2, 2);
            }
            AddTriangle(triangle);
        }


        // sides
        for (i = 0; i < sides; i++)
        {
            if (i < sides - 1)
            {
                triangle[0] = vertexList[i + 2].x;
                triangle[1] = vertexList[i + 2].y;
                triangle[2] = vertexList[i + 2].z;
                triangle[3] = vertexList[1].x;
                triangle[4] = vertexList[1].y;
                triangle[5] = vertexList[1].z;
                triangle[6] = vertexList[i + 3].x;
                triangle[7] = vertexList[i + 3].y;
                triangle[8] = vertexList[i + 3].z;
                //face->SetVertex(0, i + 2);
                //face->SetVertex(1, 1);
                //face->SetVertex(2, i + 3);
            }
            else
            {
                triangle[0] = vertexList[i + 2].x;
                triangle[1] = vertexList[i + 2].y;
                triangle[2] = vertexList[i + 2].z;
                triangle[3] = vertexList[1].x;
                triangle[4] = vertexList[1].y;
                triangle[5] = vertexList[1].z;
                triangle[6] = vertexList[2].x;
                triangle[7] = vertexList[2].y;
                triangle[8] = vertexList[2].z;
                //face->SetVertex(0, i + 2);
                //face->SetVertex(1, 1);
                //face->SetVertex(2, 2);
            }
            AddTriangle(triangle);
        }

    }

    // but this code creates a shape with clockwise winding and we want the anticlockwise default
    ReverseWinding();

//    qDebug() << "FacetedConicSegment " << GetNumTriangles() << " triangles created\n";
}

// write the object out as a POVRay string
void FacetedConicSegment::WritePOVRay(std::ostringstream &theString)
{
    bool drawDisc = false;
    if (m_Length / ((m_R1 + m_R2) / 2) < 0.001) drawDisc = true;

    dVector3 prel, p;
    prel[0] = m_OX;
    prel[1] = m_OY;
    prel[2] = m_OZ;
    prel[3] = 0;
    dMULTIPLY0_331(p, GetDisplayRotation(), prel);
    double bpx = p[0] + GetDisplayPosition()[0];
    double bpy = p[1] + GetDisplayPosition()[1];
    double bpz = p[2] + GetDisplayPosition()[2];

    prel[0] = m_OX;
    prel[1] = m_OY;
    prel[2] = m_OZ + m_Length;
    prel[3] = 0;
    dMULTIPLY0_331(p, GetDisplayRotation(), prel);
    double cpx = p[0] + GetDisplayPosition()[0];
    double cpy = p[1] + GetDisplayPosition()[1];
    double cpz = p[2] + GetDisplayPosition()[2];

    if ((std::isfinite(bpx) && std::isfinite(bpy) && std::isfinite(bpz) && std::isfinite(cpx)
            && std::isfinite(cpy) && std::isfinite(cpz)) == false) return;

    theString << "object {\n";
    if (drawDisc)
    {
        theString << "  disc {\n";
        theString << "    <" << bpx << "," << bpy << "," << bpz << ">, <" << cpx - bpx << "," << cpy - bpy << "," << cpz - bpz << ">, " << MAX(m_R1, m_R2) << "\n";
    }
    else
    {
        theString << "  cone {\n";
        theString << "    <" << bpx << "," << bpy << "," << bpz << ">, " << m_R1 << ", <" << cpx << "," << cpy << "," << cpz << ">, " << m_R2 << "\n";
    }

    // now colour
    theString << "    pigment {\n";
    theString << "      color rgbf<" << blendColour().redF() << "," << blendColour().greenF() << "," << blendColour().blueF() << "," << 1 - blendColour().alphaF() << ">\n";
    theString << "    }\n";

    theString << "  }\n";
    theString << "}\n\n";

}



