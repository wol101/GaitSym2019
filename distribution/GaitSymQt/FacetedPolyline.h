/*
 *  FacetedPolyline.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 10/08/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#ifndef FacetedPolyline_h
#define FacetedPolyline_h

#include <vector>
#include "PGDMath.h"
#include "FacetedObject.h"

class Line3D
{
public:

    Line3D() {}
    Line3D(pgd::Vector *theOrigin, pgd::Vector *theDirection)
    {
        origin = *theOrigin;
        direction = *theDirection;
        Normalize();
    }

    void Normalize()
    {
        direction.Normalize();
    }

    pgd::Vector origin;
    pgd::Vector direction;

};

class Plane3D
{
public:

    Plane3D()
    {
        a = 0;
        b = 0;
        c = 0;
        d = 0;
    }
    Plane3D(double aa, double bb, double cc, double dd)
    {
        a = aa;
        b = bb;
        c = cc;
        d = dd;
        Normalize();
    }
    Plane3D(pgd::Vector *theOrigin, pgd::Vector *theDirection1, pgd::Vector *theDirection2)
    {
        pgd::Vector normal = *theDirection1 ^ *theDirection2;
        a = normal.x;
        b = normal.y;
        c = normal.z;
        d = (-normal) * *theOrigin;
        Normalize();
    }
    Plane3D(pgd::Vector *theOrigin, pgd::Vector *theNormal)
    {
        a = theNormal->x;
        b = theNormal->y;
        c = theNormal->z;
        d = (-(*theNormal)) * *theOrigin;
    }

    pgd::Vector GetNormal()
    {
        return pgd::Vector(a, b, c);
    }
    void Normalize()
    {
        double m = 1.0 / sqrt(a * a + b * b + c * c);
        a = a * m;
        b = b * m;
        c = c * m;
        d = d * m;
    }

    double a;
    double b;
    double c;
    double d;
};

class FacetedPolyline: public FacetedObject
{
public:
    FacetedPolyline(std::vector<pgd::Vector> *polyline, double radius, int n, const QColor &colour,
                    Qt3DCore::QNode *parent = nullptr);

    void Extrude(std::vector<pgd::Vector> *polyline, std::vector<pgd::Vector> *profile);
    static bool Intersection(Line3D *line, Plane3D *plane, pgd::Vector *intersection);

};


#endif

