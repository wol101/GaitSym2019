/*
 *  FacetedCappedCylinder.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 06/02/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include "FacetedCappedCylinder.h"

// draw a capped cylinder of length l and radius r, aligned along the x axis

#ifdef USE_QT3D
FacetedCappedCylinder::FacetedCappedCylinder(double l, double r, size_t capped_cylinder_quality, const QColor &blendColour, double blendFraction, Qt3DCore::QNode *parent) : FacetedObject(parent)
#else
FacetedCappedCylinder::FacetedCappedCylinder(double l, double r, size_t capped_cylinder_quality, const QColor &blendColour, double blendFraction)
#endif
{
    setBlendColour(blendColour, blendFraction);

    double tmp, nx, ny, nz, start_nx, start_ny, a, ca, sa;
    // number of sides to the cylinder (divisible by 4):
    const size_t n = capped_cylinder_quality * 4;

    l *= 0.5;
    a = double(M_PI * 2.0) / double(n);
    sa = std::sin(a);
    ca = std::cos(a);

    AllocateMemory((n) * 2 + (n / 4) * (n) * 4);
    std::vector<pgd::Vector3> triangleStrip;
    triangleStrip.reserve(2 * (n + 1));
    pgd::Vector3 vec;

    // draw cylinder body
    ny = 1;
    nz = 0;     // normal vector = (0,ny,nz)
    for (size_t i = 0; i <= n; i++)
    {
        //glNormal3d (ny,nz,0);
        //glVertex3d (ny*r,nz*r,l);
        //glNormal3d (ny,nz,0);
        //glVertex3d (ny*r,nz*r,-l);
        vec = pgd::Vector3(ny * r, nz * r, l);
        triangleStrip.push_back(vec);
        vec = pgd::Vector3(ny * r, nz * r, -l);
        triangleStrip.push_back(vec);
        // rotate ny,nz
        tmp = ca * ny - sa * nz;
        nz = sa * ny + ca * nz;
        ny = tmp;
    }
    AddTriangleStrip(triangleStrip);

    // draw first cylinder cap
    start_nx = 0;
    start_ny = 1;
    for (size_t j = 0; j < (n / 4); j++)
    {
        // get start_n2 = rotated start_n
        double start_nx2 =  ca * start_nx + sa * start_ny;
        double start_ny2 = -sa * start_nx + ca * start_ny;
        // get n=start_n and n2=start_n2
        nx = start_nx;
        ny = start_ny;
        nz = 0;
        double nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
        triangleStrip.clear();
        for (size_t i = 0; i <= n; i++)
        {
            //glNormal3d (ny2,nz2,nx2);
            //glVertex3d (ny2*r,nz2*r,l+nx2*r);
            //glNormal3d (ny,nz,nx);
            //glVertex3d (ny*r,nz*r,l+nx*r);
            vec = pgd::Vector3(ny2 * r, nz2 * r, l + nx2 * r);
            triangleStrip.push_back(vec);
            vec = pgd::Vector3(ny * r, nz * r, l + nx * r);
            triangleStrip.push_back(vec);
            // rotate n,n2
            tmp = ca * ny - sa * nz;
            nz = sa * ny + ca * nz;
            ny = tmp;
            tmp = ca * ny2 - sa * nz2;
            nz2 = sa * ny2 + ca * nz2;
            ny2 = tmp;
        }
        AddTriangleStrip(triangleStrip);
        start_nx = start_nx2;
        start_ny = start_ny2;
    }

    // draw second cylinder cap
    start_nx = 0;
    start_ny = 1;
    for (size_t j = 0; j < (n / 4); j++)
    {
        // get start_n2 = rotated start_n
        double start_nx2 = ca * start_nx - sa * start_ny;
        double start_ny2 = sa * start_nx + ca * start_ny;
        // get n=start_n and n2=start_n2
        nx = start_nx;
        ny = start_ny;
        nz = 0;
        double nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
        triangleStrip.clear();
        for (size_t i = 0; i <= n; i++)
        {
            //glNormal3d (ny,nz,nx);
            //glVertex3d (ny*r,nz*r,-l+nx*r);
            //glNormal3d (ny2,nz2,nx2);
            //glVertex3d (ny2*r,nz2*r,-l+nx2*r);
            vec = pgd::Vector3(ny * r, nz * r, -l + nx * r);
            triangleStrip.push_back(vec);
            vec = pgd::Vector3(ny2 * r, nz2 * r, -l + nx2 * r);
            triangleStrip.push_back(vec);
            // rotate n,n2
            tmp = ca * ny - sa * nz;
            nz = sa * ny + ca * nz;
            ny = tmp;
            tmp = ca * ny2 - sa * nz2;
            nz2 = sa * ny2 + ca * nz2;
            ny2 = tmp;
        }
        AddTriangleStrip(triangleStrip);
        start_nx = start_nx2;
        start_ny = start_ny2;
    }

//    qDebug() << "FacetedCappedCylinder " << GetNumTriangles() << " triangles created\n";
}

void FacetedCappedCylinder::AddTriangleStrip(std::vector<pgd::Vector3> &triangleStrip)
{
    double triangle[9];

    for (size_t i = 2; i < triangleStrip.size(); i++)
    {
        if (i % 2 == 0)
        {
            triangle[0] = triangleStrip[i - 2].x;
            triangle[1] = triangleStrip[i - 2].y;
            triangle[2] = triangleStrip[i - 2].z;
            triangle[3] = triangleStrip[i - 1].x;
            triangle[4] = triangleStrip[i - 1].y;
            triangle[5] = triangleStrip[i - 1].z;
            triangle[6] = triangleStrip[i].x;
            triangle[7] = triangleStrip[i].y;
            triangle[8] = triangleStrip[i].z;
        }
        else
        {
            triangle[0] = triangleStrip[i - 2].x;
            triangle[1] = triangleStrip[i - 2].y;
            triangle[2] = triangleStrip[i - 2].z;
            triangle[6] = triangleStrip[i - 1].x;
            triangle[7] = triangleStrip[i - 1].y;
            triangle[8] = triangleStrip[i - 1].z;
            triangle[3] = triangleStrip[i].x;
            triangle[4] = triangleStrip[i].y;
            triangle[5] = triangleStrip[i].z;
        }
        AddTriangle(triangle);
    }
}

