/*
 *  ConvexGeom.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 27/02/2021.
 *  Copyright 2021 Bill Sellers. All rights reserved.
 *
 */

#include "ConvexGeom.h"

#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"
#include "PGDMath.h"

#include "ode/ode.h"

#include <string>

using namespace std::string_literals;

ConvexGeom::ConvexGeom(dSpaceID space, const double *planes, unsigned int planecount, const double *points, unsigned int pointcount, const unsigned int *polygons)
{
    // create the geom
    setGeomID(dCreateConvex(space, planes, planecount, points, pointcount, polygons));
    dGeomSetData(GetGeomID(), this);
}

std::string *ConvexGeom::createFromAttributes()
{
    if (Geom::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    buf.reserve(1000000);
    if (findAttribute("IndexStart"s, &buf) == nullptr) return lastErrorPtr();
    m_indexStart = GSUtil::Int(buf);
    if (findAttribute("Vertices"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, &m_vertices);
    if (findAttribute("Triangles"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Int(buf, &m_triangles);
    if (findAttribute("ReverseWinding"s, &buf)) m_reverseWinding = GSUtil::Bool(buf);
    if (m_indexStart) { for (size_t i = 0; i < m_triangles.size(); i++) { m_triangles[i] -= m_indexStart; } }
    if (m_reverseWinding) { for (size_t i = 0; i < m_triangles.size(); i += 3) { std::swap(m_triangles[i], m_triangles[i + 2]); } }
    initialiseConvexData();
    setConvex(m_planes.data(), m_planecount, m_points.data(), m_pointcount, m_polygons.data());
    return nullptr;
}

void ConvexGeom::appendToAttributes()
{
    Geom::appendToAttributes();
    std::string buf;
    buf.reserve(1000000);
    setAttribute("Type"s, "Convex"s);
    setAttribute("IndexStart"s, *GSUtil::ToString(m_indexStart, &buf));
    setAttribute("ReverseWinding"s, *GSUtil::ToString(m_reverseWinding, &buf));
    setAttribute("Vertices"s, *GSUtil::ToString(m_vertices.data(), m_vertices.size(), &buf));
    if (m_indexStart) { for (size_t i = 0; i < m_triangles.size(); i++) { m_triangles[i] += m_indexStart; } }
    if (m_reverseWinding) { for (size_t i = 0; i < m_triangles.size(); i += 3) { std::swap(m_triangles[i], m_triangles[i + 2]); } }
    setAttribute("Triangles"s, *GSUtil::ToString(m_triangles.data(), m_triangles.size(), &buf));
    if (m_reverseWinding) { for (size_t i = 0; i < m_triangles.size(); i += 3) { std::swap(m_triangles[i], m_triangles[i + 2]); } }
    if (m_indexStart) { for (size_t i = 0; i < m_triangles.size(); i++) { m_triangles[i] -= m_indexStart; } }
    return;
}


void ConvexGeom::setConvex(const double *planes, unsigned int planecount, const double *points, unsigned int pointcount, const unsigned int *polygons)
{
    dGeomSetConvex(GetGeomID(), planes, planecount, points, pointcount, polygons);
}

void ConvexGeom::initialiseConvexData()
{
    // set the limits
    m_pointcount = static_cast<unsigned int>(m_vertices.size() / 3);
    m_planecount = static_cast<unsigned int>(m_triangles.size() / 3);
    // copy the vertices into m_points
    m_points.clear();
    m_points.reserve(m_pointcount * 3);
    for (size_t i = 0; i < m_vertices.size(); i++) m_points.push_back(m_vertices[i]);
    // copy the triangles into m_polygons with the correct vertex count
    m_polygons.clear();
    m_polygons.reserve(m_planecount * 4);
    for (size_t i = 0; i < m_triangles.size();)
    {
        m_polygons.push_back(3);
        m_polygons.push_back(m_triangles[i++]);
        m_polygons.push_back(m_triangles[i++]);
        m_polygons.push_back(m_triangles[i++]);
    }
    // calculate and write the planes
    // Transform a parametric plane form to the cartesian form
    //
    // We are given a plane in the parametric form x = p + ru + sv and
    // want to transform it to the cartesian form ax+ by + cz = d.
    // First we need to calculate the normal vector equation of the plane by using the cross product:
    //
    //    n = cross(u, v)
    //
    // n may need normalising depending on how u and v are calculated
    // We calculate d as dot(n, p) and a, b, and c are the components of the n vector:
    //
    // dot(n, x) = dot(n, p)
    // n1x + n2y + n3z = dot(n, p)
    // ax + by + cz = d
    m_planes.clear();
    m_planes.reserve((m_triangles.size() / 3) * 4);
    pgd::Vector3 v1, v2, v3;
    pgd::Vector3 p, u, v;
    pgd::Vector3 n;
    double d;
    for (size_t i = 0; i < m_triangles.size();)
    {
        v1.Set(m_vertices[m_triangles[i] * 3], m_vertices[m_triangles[i] * 3 + 1], m_vertices[m_triangles[i] * 3 + 2]);
        i++;
        v2.Set(m_vertices[m_triangles[i] * 3], m_vertices[m_triangles[i] * 3 + 1], m_vertices[m_triangles[i] * 3 + 2]);
        i++;
        v3.Set(m_vertices[m_triangles[i] * 3], m_vertices[m_triangles[i] * 3 + 1], m_vertices[m_triangles[i] * 3 + 2]);
        i++;
        // for a triangle v1, v2, v3, if the vector U = v2 - v1 and the vector V = v3 - v1
        // then the normal N = U x V
        // Note: the normal is the same if V = v3 - v2
        p = v1;
        u = v2 - v1;
        v = v3 - v1;
        n = pgd::Cross(u, v);
        n.Normalize();
        d = pgd::Dot(n, p);
        m_planes.push_back(n.x);
        m_planes.push_back(n.y);
        m_planes.push_back(n.z);
        m_planes.push_back(d);
        // std::cerr << n.x << " " << n.y << " " << n.z << " " << d << "\n";
    }
}

std::vector<int> *ConvexGeom::triangles()
{
    return &m_triangles;
}

std::vector<double> *ConvexGeom::vertices()
{
    return &m_vertices;
}

