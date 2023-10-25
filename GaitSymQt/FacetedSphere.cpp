/*
 *  FacetedSphere.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 03/01/2006.
 *  Copyright 2006 Bill Sellers. All rights reserved.
 *
 */

// this code largely based on code by Jon Leech from:
// http://www.gamedev.net/reference/articles/article427.asp
/*
 * sphere - generate a polygon mesh approximating a sphere by
 *  recursive subdivision. First approximation is an octahedron;
 *  each level of refinement increases the number of polygons by
 *  a factor of 4.
 * Level 3 (128 polygons) is a good tradeoff if gouraud
 *  shading is used to render the database.
 *
 * Usage: sphere [level]
 *      level is an integer >= 1 setting the recursion level (default 1).
 *
 * Notes:
 *
 *  The triangles are generated with vertices in clockwise order as
 *  viewed from the outside in a right-handed coordinate system.
 *  To reverse the order, compile with COUNTERCLOCKWISE defined.
 *
 *  Shared vertices are not retained, so numerical errors may produce
 *  cracks between polygons at high subdivision levels.
 *
 *  The subroutines print_object() and print_triangle() should
 *  be changed to generate whatever the desired database format is.
 *  If UNC is defined, a PPHIGS text archive is generated.
 *
 * Jon Leech 3/24/89
 */

#define COUNTERCLOCKWISE

#include <stdio.h>
#include <cmath>
#include <ode/ode.h>
#include <sstream>

#include "FacetedSphere.h"

typedef struct
{
    double  x, y, z;
} point;

typedef struct
{
    point     pt[3];    /* Vertices of triangle */
    double    area;     /* Unused; might be used for adaptive subdivision */
} triangle;

typedef struct
{
    size_t   npoly;    /* # of polygons in object */
    triangle *poly;     /* Polygons in no particular order */
} object;

/* Six equidistant points lying on the unit sphere */
#define XPLUS {  1,  0,  0 }    /*  X */
#define XMIN  { -1,  0,  0 }    /* -X */
#define YPLUS {  0,  1,  0 }    /*  Y */
#define YMIN  {  0, -1,  0 }    /* -Y */
#define ZPLUS {  0,  0,  1 }    /*  Z */
#define ZMIN  {  0,  0, -1 }    /* -Z */

/* Forward declarations */
static point *normalize( point *p );
static point *midpoint( point *a, point *b );

#ifdef USE_QT3D
FacetedSphere::FacetedSphere(double radius, size_t maxlevels, const QColor &blendColour, double blendFraction, Qt3DCore::QNode *parent) : FacetedObject(parent)
#else
FacetedSphere::FacetedSphere(double radius, size_t maxlevels, const QColor &blendColour, double blendFraction)
#endif
{
    setBlendColour(blendColour, blendFraction);

    /* Vertices of a unit octahedron */
    triangle octahedron[] =
    {
        {{ XPLUS, ZPLUS, YPLUS }, 0.0},
        {{ YPLUS, ZPLUS, XMIN  }, 0.0},
        {{ XMIN, ZPLUS, YMIN  }, 0.0},
        {{ YMIN, ZPLUS, XPLUS }, 0.0},
        {{ XPLUS, YPLUS, ZMIN  }, 0.0},
        {{ YPLUS, XMIN, ZMIN  }, 0.0},
        {{ XMIN, YMIN, ZMIN  }, 0.0},
        {{ YMIN, XPLUS, ZMIN  }, 0.0}
    };

    /* An octahedron */
    object oct =
    {
        sizeof(octahedron) / sizeof(octahedron[0]),
        &octahedron[0]
    };

    object *oldObject, *newObject;
    size_t     i, level;

    if (maxlevels < 1) maxlevels = 1;
    m_Level = maxlevels;
    m_Radius = radius;

#ifdef COUNTERCLOCKWISE
    /* Reverse order of points in each triangle */
    for (i = 0; i < oct.npoly; i++)
    {
        point tmp;
        tmp = oct.poly[i].pt[0];
        oct.poly[i].pt[0] = oct.poly[i].pt[2];
        oct.poly[i].pt[2] = tmp;
    }
#endif

    oldObject = &oct;

    /* Subdivide each starting triangle (maxlevels - 1) times */
    for (level = 1; level < maxlevels; level++)
    {
        /* Allocate a new object */
        newObject = static_cast<object *>(malloc(sizeof(object)));
        if (newObject == nullptr)
        {
            fprintf(stderr, "FacetedSphere: Out of memory on subdivision level %zud\n", level);
            exit(1);
        }
        newObject->npoly = oldObject->npoly * 4;

        /* Allocate 4* the number of points in the current approximation */
        newObject->poly  = static_cast<triangle *>(malloc(newObject->npoly * sizeof(triangle)));
        if (newObject->poly == nullptr)
        {
            fprintf(stderr, "FacetedSphere: Out of memory on subdivision level %zu\n", level);
            exit(1);
        }

        /* Subdivide each polygon in the old approximation and normalize
            *  the new points thus generated to lie on the surface of the unit
            *  sphere.
            * Each input triangle with vertices labelled [0,1,2] as shown
            *  below will be turned into four new triangles:
            *
            *                      Make new points
            *                          a = (0+2)/2
            *                          b = (0+1)/2
            *                          c = (1+2)/2
            *        1
            *       /\             Normalize a, b, c
            *      /  \
            *    b/____\ c         Construct new triangles
            *    /\    /\              [0,b,a]
            *   /  \  /  \             [b,1,c]
            *  /____\/____\            [a,b,c]
            * 0      a     2           [a,c,2]
            */
        for (i = 0; i < oldObject->npoly; i++)
        {
            triangle *oldt = &oldObject->poly[i];
            triangle *newt = &newObject->poly[i * 4];
            point a, b, c;

            a = *normalize(midpoint(&oldt->pt[0], &oldt->pt[2]));
            b = *normalize(midpoint(&oldt->pt[0], &oldt->pt[1]));
            c = *normalize(midpoint(&oldt->pt[1], &oldt->pt[2]));

            newt->pt[0] = oldt->pt[0];
            newt->pt[1] = b;
            newt->pt[2] = a;
            newt++;

            newt->pt[0] = b;
            newt->pt[1] = oldt->pt[1];
            newt->pt[2] = c;
            newt++;

            newt->pt[0] = a;
            newt->pt[1] = b;
            newt->pt[2] = c;
            newt++;

            newt->pt[0] = a;
            newt->pt[1] = c;
            newt->pt[2] = oldt->pt[2];
        }

        if (level > 1)
        {
            free(oldObject->poly);
            free(oldObject);
        }

        /* Continue subdividing new triangles */
        oldObject = newObject;
    }

    /* Print out resulting approximation */
    //print_object(oldObject, maxlevels);

    // add the faces to the Faceted Object
    double vertex[9];
    AllocateMemory(oldObject->npoly);
    for (i = 0; i < oldObject->npoly; i++)
    {
        vertex[0] = oldObject->poly[i].pt[0].x;
        vertex[1] = oldObject->poly[i].pt[0].y;
        vertex[2] = oldObject->poly[i].pt[0].z;
        vertex[3] = oldObject->poly[i].pt[1].x;
        vertex[4] = oldObject->poly[i].pt[1].y;
        vertex[5] = oldObject->poly[i].pt[1].z;
        vertex[6] = oldObject->poly[i].pt[2].x;
        vertex[7] = oldObject->poly[i].pt[2].y;
        vertex[8] = oldObject->poly[i].pt[2].z;
        AddTriangle(vertex);
    }

    free(oldObject->poly);
    free(oldObject);

    Scale(radius, radius, radius);

//    qDebug() << "FacetedSphere " << GetNumTriangles() << " triangles created\n";
}

// write the object out as a POVRay string
void FacetedSphere::WritePOVRay(std::ostringstream &theString)
{
    theString << "object {\n";
    theString << "  sphere {\n";
    theString << "    <" << GetDisplayPosition()[0] << "," << GetDisplayPosition()[1] << "," << GetDisplayPosition()[2] << ">, " << m_Radius << "\n";

    // now colour
    theString << "    pigment {\n";
    theString << "      color rgbf<" << blendColour().redF() << "," << blendColour().greenF() << "," << blendColour().blueF() << "," << 1 - blendColour().alphaF() << ">\n";
    theString << "    }\n";

    theString << "  }\n";
    theString << "}\n\n";
}

size_t FacetedSphere::EstimateLevel(size_t requestedFaces, size_t *actualFaces)
{
    // basic octohedron has 8 faces (level 1)
    // and this is doubled each level
    // n = 8 x 2^(level - 1)
    double actualLevel = 1 + (std::log(double(requestedFaces)) - std::log(8.0)) / std::log(2);
    size_t level = size_t(std::ceil(actualLevel));
    if (actualFaces) *actualFaces = 8 * size_t(std::pow(2.0, double(level)) + 0.5);
    return level;
}

/* Normalize a point p */
point *normalize(point *p)
{
    static point r;
    double mag;

    r = *p;
    mag = r.x * r.x + r.y * r.y + r.z * r.z;
    if (mag != 0.0)
    {
        mag = 1.0 / sqrt(mag);
        r.x *= mag;
        r.y *= mag;
        r.z *= mag;
    }

    return &r;
}

/* Return the average of two points */
point *midpoint(point *a, point *b)
{
    static point r;

    r.x = (a->x + b->x) * 0.5;
    r.y = (a->y + b->y) * 0.5;
    r.z = (a->z + b->z) * 0.5;

    return &r;
}


