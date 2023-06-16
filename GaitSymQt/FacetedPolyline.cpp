/*
 *  FacetedPolyline.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 10/08/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

// Uses algorithms described in Geometric Tools for Computer Graphics, Scheider & Eberly 2003

#include "FacetedPolyline.h"

#include "../glextrusion/gle.h"

#include <iostream>
#include <cmath>
#include <memory>

FacetedPolyline::FacetedPolyline(std::vector<pgd::Vector3> *polyline, double radius, size_t n, const QColor &blendColour, double blendFraction, bool internal)
{
    setBlendColour(blendColour, blendFraction);
    if (internal)
    {
        std::vector<pgd::Vector3> profile;
        AllocateMemory(n * (polyline->size() * 2 + 2));

        // need to add extra tails to the polyline for direction padding
        std::vector<pgd::Vector3> newPolyline;
        newPolyline.reserve(polyline->size() + 2);
        pgd::Vector3 v0 = (*polyline)[1] - (*polyline)[0];
        pgd::Vector3 v1 = (*polyline)[0] - v0;
        newPolyline.push_back(v1);
        for (size_t i = 0; i < polyline->size(); i++) newPolyline.push_back((*polyline)[i]);
        v0 = (*polyline)[polyline->size() - 1] - (*polyline)[polyline->size() - 2];
        v1 = (*polyline)[polyline->size() - 1] + v0;
        newPolyline.push_back(v1);

        // create the profile
        double delTheta = 2 * M_PI / n;
        double theta = M_PI / 2;
        for (size_t i = 0; i < n; i++)
        {
            v0.x = cos(theta) * radius;
            v0.y = sin(theta) * radius;
            v0.z = 0;
            theta -= delTheta;
            profile.push_back(v0);
        }

        Extrude(&newPolyline, &profile);
    }
    else
    {
        gleSetNumSides(int(n));
        size_t nPoints = polyline->size() + 2;
        auto point_array = std::make_unique<gleDouble[][3]>(nPoints);
        pgd::Vector3 v0 = (*polyline)[1] - (*polyline)[0];
        pgd::Vector3 v1 = (*polyline)[0] - v0;
        point_array[0][0] = v1.x; point_array[0][1] = v1.y; point_array[0][2] = v1.z;
        for (size_t i = 1 ; i < nPoints - 1; i++) { point_array[i][0] = (*polyline)[i - 1].x; point_array[i][1] = (*polyline)[i - 1].y; point_array[i][2] = (*polyline)[i - 1].z; }
        v0 = (*polyline)[polyline->size() - 1] - (*polyline)[polyline->size() - 2];
        v1 = (*polyline)[polyline->size() - 1] + v0;
        point_array[nPoints][0] = v1.x; point_array[nPoints][1] = v1.y; point_array[nPoints][2] = v1.z;
        auto color_array = std::make_unique<gleColor[]>(nPoints);
        float r = blendColour.redF();
        float g = blendColour.greenF();
        float b = blendColour.blueF();
        for (size_t i = 0 ; i < nPoints; i++) { color_array[i][0] = r; color_array[i][1] = g; color_array[i][2] = b; }
        glePolyCylinder(int(nPoints),           /* num points in polyline */
                        point_array.get(),      /* polyline vertces */
                        color_array.get(),      /* colors at polyline verts */
                        radius);                /* radius of polycylinder */
//        auto color_array = std::make_unique<gleColor4f[]>(nPoints);
//        float r = blendColour.redF();
//        float g = blendColour.greenF();
//        float b = blendColour.blueF();
//        float a = blendColour.alphaF();
//        for (size_t i = 0 ; i < nPoints; i++) { color_array[i][0] = r; color_array[i][1] = g; color_array[i][2] = b; color_array[i][3] = a; }
//        glePolyCylinder_c4f(int(nPoints),       /* num points in polyline */
//                        point_array.get(),      /* polyline vertces */
//                        color_array.get(),      /* colors at polyline verts */
//                        radius);                /* radius of polycylinder */
        exit(1);
    }

}

// extrude profile along a poly line using sharp corners
// profile is a 2D shape with z = 0 for all values.
// polyline needs to have no parallel neighbouring segements
// anti-clockwise winding assumed (I think)
// first and last point of polyline used for direction only!
void FacetedPolyline::Extrude(std::vector<pgd::Vector3> *polyline, std::vector<pgd::Vector3> *profile)
{
    unsigned int i, j;
    Line3D line;
    pgd::Vector3 v1, v2, p1, p2;
    const double epsilon = 0.000001;
    const double epsilon2 = epsilon * epsilon;
    auto polygon = std::make_unique<double []>(profile->size() * 3);
    auto polygonNormal = std::make_unique<double []>(profile->size() * 3);

    // define the planes of the joins
    std::vector<Plane3D> joinPlanes;
    joinPlanes.reserve(polyline->size());
    Plane3D plane;
    for (i = 1; i < (*polyline).size() - 1; i++)
    {
        v1 = (*polyline)[i] - (*polyline)[i - 1];
        v2 = (*polyline)[i + 1] - (*polyline)[i];
        v1.Normalize();
        v2.Normalize();
        p1 = v1 - v2;
        if (p1.Magnitude2() > epsilon2)   // not parallel so use two vector form of plane
        {
            p2 = v1 ^ v2;
            plane = Plane3D(&(*polyline)[i], &p1, &p2);
        }
        else     // parallel so use normal-point form
        {
            plane = Plane3D(&(*polyline)[i], &v1);
        }

        joinPlanes.push_back(plane);
    }

    // define the rotated (*profile) for the first line segment
    // note for a truly generic routine you need two rotations to allow an up vector to be defined
    pgd::Vector3 zVec(0, 0, 1);
    v1 = (*polyline)[1] - (*polyline)[0];
    pgd::Quaternion q = pgd::FindRotation(zVec, v1);
    std::vector<pgd::Vector3> rotatedProfile;
    rotatedProfile.reserve(polyline->size());
    for (i = 0; i < (*profile).size(); i++)
    {
        pgd::Vector3 v = pgd::QVRotate(q, (*profile)[i]);
        rotatedProfile.push_back(v);
    }

    // find the intersections on the join planes
    std::vector<pgd::Vector3> vertexList;
    vertexList.reserve(rotatedProfile.size() * joinPlanes.size());
    for (i = 0; i < rotatedProfile.size(); i++)
    {
        v2 = (*polyline)[0] + rotatedProfile[i];
        line = Line3D(&v2, &v1);
        for (j = 0; j < joinPlanes.size(); j++)
        {
            if (Intersection(&line, &joinPlanes[j], &v2))
            {
                vertexList.push_back(v2);
                if (j < joinPlanes.size() - 1)
                {
                    p1 = (*polyline)[j + 2] - (*polyline)[j + 1];
                    line = Line3D(&v2, &p1);
                }
            }
            else
            {
                qDebug() << "Error finding line & plane intersection" << "\n";
                return;
            }
        }
    }

    // now construct faces
    pgd::Vector3 centroid1, centroid2, normal;
    for (i = 0; i < rotatedProfile.size(); i++)
    {
        for (j = 0; j < joinPlanes.size() - 1; j++)
        {
            centroid1 = polyline->at(j + 1);
            centroid2 = polyline->at(j + 2);
            if (i < rotatedProfile.size() - 1)
            {
                polygon[0] = vertexList[j + joinPlanes.size() * i].x;
                polygon[1] = vertexList[j + joinPlanes.size() * i].y;
                polygon[2] = vertexList[j + joinPlanes.size() * i].z;
                polygon[3] = vertexList[1 + j + joinPlanes.size() * i].x;
                polygon[4] = vertexList[1 + j + joinPlanes.size() * i].y;
                polygon[5] = vertexList[1 + j + joinPlanes.size() * i].z;
                polygon[6] = vertexList[1 + j + joinPlanes.size() * (i + 1)].x;
                polygon[7] = vertexList[1 + j + joinPlanes.size() * (i + 1)].y;
                polygon[8] = vertexList[1 + j + joinPlanes.size() * (i + 1)].z;
                polygon[9] = vertexList[j + joinPlanes.size() * (i + 1)].x;
                polygon[10] = vertexList[j + joinPlanes.size() * (i + 1)].y;
                polygon[11] = vertexList[j + joinPlanes.size() * (i + 1)].z;
            }
            else
            {
                polygon[0] = vertexList[j + joinPlanes.size() * i].x;
                polygon[1] = vertexList[j + joinPlanes.size() * i].y;
                polygon[2] = vertexList[j + joinPlanes.size() * i].z;
                polygon[3] = vertexList[1 + j + joinPlanes.size() * i].x;
                polygon[4] = vertexList[1 + j + joinPlanes.size() * i].y;
                polygon[5] = vertexList[1 + j + joinPlanes.size() * i].z;
                polygon[6] = vertexList[1 + j].x;
                polygon[7] = vertexList[1 + j].y;
                polygon[8] = vertexList[1 + j].z;
                polygon[9] = vertexList[j].x;
                polygon[10] = vertexList[j].y;
                polygon[11] = vertexList[j].z;
            }
            normal = pgd::Vector3(polygon[0], polygon[1], polygon[2]) - centroid1;
            normal.Normalize();
            polygonNormal[0] = normal.x;
            polygonNormal[1] = normal.y;
            polygonNormal[2] = normal.z;
            normal = pgd::Vector3(polygon[3], polygon[4], polygon[5]) - centroid2;
            normal.Normalize();
            polygonNormal[3] = normal.x;
            polygonNormal[4] = normal.y;
            polygonNormal[5] = normal.z;
            normal = pgd::Vector3(polygon[6], polygon[7], polygon[8]) - centroid2;
            normal.Normalize();
            polygonNormal[6] = normal.x;
            polygonNormal[7] = normal.y;
            polygonNormal[8] = normal.z;
            normal = pgd::Vector3(polygon[9], polygon[10], polygon[11]) - centroid1;
            normal.Normalize();
            polygonNormal[9] = normal.x;
            polygonNormal[10] = normal.y;
            polygonNormal[11] = normal.z;
            this->AddPolygon(polygon.get(), 4, polygonNormal.get());
        }
    }

    // end caps
    for (i = 0; i < rotatedProfile.size(); i++)
    {
        polygon[i * 3] = vertexList[joinPlanes.size() * i].x;
        polygon[i * 3 + 1] = vertexList[joinPlanes.size() * i].y;
        polygon[i * 3 + 2] = vertexList[joinPlanes.size() * i].z;
    }
    this->AddPolygon(polygon.get(), rotatedProfile.size());
    for (i = 0; i < rotatedProfile.size(); i++)
    {
        polygon[i * 3] = vertexList[joinPlanes.size() - 1 + joinPlanes.size() * (rotatedProfile.size() - i - 1)].x;
        polygon[i * 3 + 1] = vertexList[joinPlanes.size() - 1 + joinPlanes.size() * (rotatedProfile.size() - i - 1)].y;
        polygon[i * 3 + 2] = vertexList[joinPlanes.size() - 1 + joinPlanes.size() * (rotatedProfile.size() - i - 1)].z;
    }
    this->AddPolygon(polygon.get(), rotatedProfile.size());
}

// find intersection of line and plane
// returns true on success, false if no intersection
bool FacetedPolyline::Intersection(Line3D *line, Plane3D *plane, pgd::Vector3 *intersection)
{
    double denominator = line->direction * plane->GetNormal();
    const double epsilon = 0.000001;
    double t;

    if (fabs(denominator) < epsilon)
    {
        // line and plane very close to parallel so they probably don't meet
        // but perhaps the origin is in the plane
        if (fabs(line->origin.x * plane->a + line->origin.y * plane->b + line->origin.z * plane->c + plane->d) > epsilon)
        {
            t = 0;
            *intersection = line->origin;
            return true;
        }
        else
        {
            return false;
        }
    }

    // compute intersection

    t = -(plane->a * line->origin.x + plane->b * line->origin.y + plane->c * line->origin.z + plane->d);
    t = t / denominator;
    *intersection = line->origin + t * line->direction;

    return true;

}

