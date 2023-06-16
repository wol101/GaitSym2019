/*
 *  FluidSac.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 02/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "FluidSac.h"
#include "Marker.h"
#include "GSUtil.h"
#include "Body.h"

#include "pystring.h"

#include <map>
#include <algorithm>
#include <sstream>

using namespace std::string_literals;

FluidSac::FluidSac()
{
}

void FluidSac::calculateVolume()
{
    for (size_t i = 0; i < m_markerList.size(); i++) m_vertexList[i] = m_markerList[i]->GetWorldPosition();
    double currentVolume = volumeOfMesh(m_triangleList, m_vertexList);
    double deltaT = simulation()->GetTimeIncrement() / 1e-6;
    std::vector<pgd::Vector3> deltaList(m_markerList.size());
    for (size_t i = 0; i < m_markerList.size(); i++) deltaList[i] = m_markerList[i]->GetWorldLinearVelocity() * deltaT; // using the marker velocities gives a semi-implicit solution which should be more stable
    std::vector<pgd::Vector3> vertexList1 = m_vertexList;
    std::vector<pgd::Vector3> vertexList2 = m_vertexList;
    for (size_t i = 0; i < m_markerList.size(); i++) vertexList1[i] -= deltaList[i];
    for (size_t i = 0; i < m_markerList.size(); i++) vertexList2[i] += deltaList[i];
    double volume1 = volumeOfMesh(m_triangleList, vertexList1);
    double volume2 = volumeOfMesh(m_triangleList, vertexList1);
    setSacVolume(currentVolume);
    setDotSacVolume((volume2 - volume1) / (2 * deltaT));
}


void FluidSac::calculateLoadsOnMarkers()
{
    pgd::Vector3 v01, v12, v20;
    m_pointForceList.resize(m_triangleList.size() * 3);
    size_t pointListIndex = 0;
    for (size_t i = 0; i < m_triangleList.size(); i++)
    {
        // for each triangle calculate the equivalent force and torque on each marker.
        FluidSac::Triangle *it = &m_triangleList[i];
        areaCentroidNormal(m_vertexList[it->v0], m_vertexList[it->v1], m_vertexList[it->v2], &it->area, &it->centroid, &it->normal);

        // now rotate the system so that the normal is aligned to the z axis
        pgd::Quaternion r = pgd::FindRotation(it->normal, pgd::Vector3(0, 0, 1));
        pgd::Vector3 v0 = pgd::QVRotate(r, m_vertexList[it->v0]);
        pgd::Vector3 v1 = pgd::QVRotate(r, m_vertexList[it->v1]);
        pgd::Vector3 v2 = pgd::QVRotate(r, m_vertexList[it->v2]);
        pgd::Vector3 centroid = pgd::QVRotate(r, it->centroid);

        // now we can use the Z=0 triangle formulae
        // the triangle is in the z=0 plane
        // the applied vertical force is F
        // the reaction forces are R0, R1, R2
        // the position of the applied force is specified as x, y
        // the positions of the vertices are x0,y0, x1,y1, x2,y2

        // R0 = (F*(x*y1 - x1*y - x*y2 + x2*y + x1*y2 - x2*y1))/(x0*y1 - x1*y0 - x0*y2 + x2*y0 + x1*y2 - x2*y1)
        // R1 = -(F*(x*y0 - x0*y - x*y2 + x0*y2 + x2*y - x2*y0))/(x0*y1 - x1*y0 - x0*y2 + x2*y0 + x1*y2 - x2*y1)
        // R2 = (F*(x*y0 - x0*y - x*y1 + x0*y1 + x1*y - x1*y0))/(x0*y1 - x1*y0 - x0*y2 + x2*y0 + x1*y2 - x2*y1)

        double x = centroid.x;
        double y = centroid.y;
        double x0 = v0.x;
        double x1 = v1.x;
        double x2 = v2.x;
        double y0 = v0.y;
        double y1 = v1.y;
        double y2 = v2.y;
        double F = it->area * m_pressure;
        double denom = (x0*y1 - x1*y0 - x0*y2 + x2*y0 + x1*y2 - x2*y1);
        double R0 = (F*(x*y1 - x1*y - x*y2 + x2*y + x1*y2 - x2*y1)) / denom;
        double R1 = -(F*(x*y0 - x0*y - x*y2 + x0*y2 + x2*y - x2*y0)) / denom;
        double R2 = (F*(x*y0 - x0*y - x*y1 + x0*y1 + x1*y - x1*y0)) / denom;

        // and add the reaction forces to the body
        m_pointForceList[pointListIndex].body = m_markerList[it->v0]->GetBody();
        std::copy_n(m_vertexList[it->v0].data(), 3, m_pointForceList[pointListIndex].point);
        std::copy_n((it->normal * R0).data(), 3, m_pointForceList[pointListIndex].vector);
        pointListIndex++;
        m_pointForceList[pointListIndex].body = m_markerList[it->v1]->GetBody();
        std::copy_n(m_vertexList[it->v1].data(), 3, m_pointForceList[pointListIndex].point);
        std::copy_n((it->normal * R1).data(), 3, m_pointForceList[pointListIndex].vector);
        pointListIndex++;
        m_pointForceList[pointListIndex].body = m_markerList[it->v2]->GetBody();
        std::copy_n(m_vertexList[it->v2].data(), 3, m_pointForceList[pointListIndex].point);
        std::copy_n((it->normal * R2).data(), 3, m_pointForceList[pointListIndex].vector);
        pointListIndex++;
    }
}

bool FluidSac::isGoodMesh(const std::vector<FluidSac::Triangle> &triangleList, const std::vector<Marker *> & /* markerList */)
{
    // for a water tight mesh edges should be in pairs and the direction should be reversed
    // first store the edges in a multimap using the start vertex as key
    std::multimap <size_t, size_t> edgeList;
    for (auto &&it : triangleList)
    {
        edgeList.insert(std::make_pair(it.v0, it.v1));
        edgeList.insert(std::make_pair(it.v1, it.v2));
        edgeList.insert(std::make_pair(it.v2, it.v0));
    }
    // then iterate through the edges
    for (auto &&it : edgeList)
    {
        // get all the edges that start with the end vertex
        auto matched = edgeList.equal_range(it.second);
        size_t count = 0;
        for (auto matchedIt = matched.first; matchedIt != matched.second; matchedIt++)
        {
            if (matchedIt->second == it.first) count++;
        }
        // there should be exactly one reversed edge
        if (count != 1) return false;
    }
    return true;
}

double FluidSac::signedVolumeOfTriangle(pgd::Vector3 p1, pgd::Vector3 p2, pgd::Vector3 p3)
{
    return p1.Dot(p2.Cross(p3)) / 6.0;
}

double FluidSac::volumeOfMesh(const std::vector<FluidSac::Triangle> &triangleList, const std::vector<pgd::Vector3> &vectorList)
{
    double volumeSum = 0;
    for (auto &&it : triangleList)
    {
        volumeSum += signedVolumeOfTriangle(vectorList[it.v0], vectorList[it.v1], vectorList[it.v2]);
    }
    return std::abs(volumeSum);
}

double FluidSac::area(std::vector<std::pair<double, double>> points)
{
    // this is the shoelace formula for calculating the area of an arbitrary polygon
    // defined by the coordinates of its vertices
    double leftSum = 0.0;
    double rightSum = 0.0;

    for (size_t i = 0; i < points.size(); ++i)
    {
        size_t j = (i + 1) % points.size();
        leftSum  += points[i].first * points[j].second;
        rightSum += points[j].first * points[i].second;
    }

    return 0.5 * abs(leftSum - rightSum);
}

void FluidSac::areaCentroid(std::vector<std::pair<double, double>> points, double *area, std::pair<double, double> *centroid)
{
    // this is the shoelace formula for calculating the area of an arbitrary polygon
    // defined by the coordinates of its vertices
    double leftSum = 0.0;
    double rightSum = 0.0;
    double cxSum = 0.0;
    double cySum = 0.0;

    for (size_t i = 0; i < points.size(); ++i)
    {
        size_t j = (i + 1) % points.size();
        leftSum  += points[i].first * points[j].second;
        rightSum += points[j].first * points[i].second;
        double t = points[i].first * points[j].second - points[j].first * points[i].second;
        cxSum += (points[i].first + points[j].first) * t;
        cySum += (points[i].second + points[j].second) * t;
    }

    *area = 0.5 * abs(leftSum - rightSum);
    centroid->first = cxSum / (*area * 6.0);
    centroid->second = cySum / (*area * 6.0);
}

void FluidSac::areaCentroidNormal(const pgd::Vector3 &v0, const pgd::Vector3 &v1, const pgd::Vector3 &v2, double *area, pgd::Vector3 *centroid, pgd::Vector3 *normal)
{
    *centroid = (v0 + v1 + v2) / 3.0; // centroid is easy for triangles
    pgd::Vector3 edge0 = v1 - v0;
    pgd::Vector3 edge1 = v2 - v1;
    pgd::Vector3 crossProduct = edge0.Cross(edge1);
    double crossProductMagnitude = crossProduct.Magnitude(); // cross product magnitude is the area of the parallelogram
    *area = crossProduct.Magnitude() / 2; // and the area of the triangle is half the area of the prallelogram
    *normal = crossProduct / crossProductMagnitude;
}

void FluidSac::areaCentroidNormal(const pgd::Vector3 &v0, const pgd::Vector3 &v1, const pgd::Vector3 &v2, const pgd::Vector3 &v3, double *area, pgd::Vector3 *centroid, pgd::Vector3 *normal)
{
    // triangle 1
    double area1;
    pgd::Vector3 centroid1;
    pgd::Vector3 normal1;
    areaCentroidNormal(v0, v1, v2, &area1, &centroid1, &normal1);
    // triangle 2
    double area2;
    pgd::Vector3 centroid2;
    pgd::Vector3 normal2;
    areaCentroidNormal(v0, v2, v3, &area2, &centroid2, &normal2);
    // check normals
    assert(normal1.Dot(normal2) > 0.9999999999);
    *normal = normal1;
    *area = area1 + area2;
    *centroid = (centroid1 * area1 + centroid2 * area2) / *area;
}

double FluidSac::sacVolume() const
{
    return m_sacVolume;
}

double FluidSac::pressure() const
{
    return m_pressure;
}

const std::vector<PointForce> &FluidSac::pointForceList() const
{
    return m_pointForceList;
}

const std::vector<FluidSac::Triangle> &FluidSac::triangleList() const
{
    return m_triangleList;
}

size_t FluidSac::numTriangles() const
{
    return m_triangleList.size();
}

void FluidSac::triangleVertices(size_t triangleIndex, double vertices[9]) const
{
    const Triangle *tri = &m_triangleList[triangleIndex];
    vertices[0] = m_vertexList[tri->v0].x;
    vertices[1] = m_vertexList[tri->v0].y;
    vertices[2] = m_vertexList[tri->v0].z;
    vertices[3] = m_vertexList[tri->v1].x;
    vertices[4] = m_vertexList[tri->v1].y;
    vertices[5] = m_vertexList[tri->v1].z;
    vertices[6] = m_vertexList[tri->v2].x;
    vertices[7] = m_vertexList[tri->v2].y;
    vertices[8] = m_vertexList[tri->v2].z;
}

void FluidSac::LateInitialisation()
{
    this->calculateVolume();
    // m_lastSacVolume = m_sacVolume;
    this->calculatePressure();
    this->calculateLoadsOnMarkers();
}

std::string *FluidSac::createFromAttributes()
{
    if (NamedObject::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    buf.reserve(1000000);

    if (findAttribute("NumMarkers"s, &buf) == nullptr) return lastErrorPtr();
    size_t numMarkers = size_t(GSUtil::Int(buf));
    if (findAttribute("MarkerIDList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> markerNames;
    pystring::split(buf, markerNames);
    if (numMarkers != markerNames.size())
    {
        setLastError("FLUIDSAC ID=\""s + name() +"\" NumMarkers does not match number found in MarkerIDList"s);
        return lastErrorPtr();
    }
    m_markerList.clear();
    m_markerList.reserve(markerNames.size());
    for (size_t i = 0; i < markerNames.size(); i++)
    {
        auto it = this->simulation()->GetMarkerList()->find(markerNames[i]);
        if (it == this->simulation()->GetMarkerList()->end())
        {
            setLastError("FLUIDSAC ID=\""s + name() +"\" Marker ID=\""s + markerNames[i] + "\" not found"s);
            return lastErrorPtr();
        }
        m_markerList.push_back(it->second.get());
    }
    if (findAttribute("NumTriangles"s, &buf) == nullptr) return lastErrorPtr();
    size_t numTriangles = size_t(GSUtil::Int(buf));
    if (findAttribute("TriangleIndexList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> markerIndices;
    pystring::split(buf, markerIndices);
    if (numTriangles * 3 != markerIndices.size())
    {
        setLastError("FLUIDSAC ID=\""s + name() +"\" NumTriangles does not match number found in TriangleIndexList"s);
        return lastErrorPtr();
    }
    m_triangleList.clear();
    m_triangleList.resize(numTriangles);
    for (size_t i = 0; i < numTriangles; i++)
    {
        m_triangleList[i].v0 = size_t(GSUtil::Int(markerIndices[i * 3 + 0]));
        m_triangleList[i].v1 = size_t(GSUtil::Int(markerIndices[i * 3 + 1]));
        m_triangleList[i].v2 = size_t(GSUtil::Int(markerIndices[i * 3 + 2]));
        m_triangleList[i].area = 0;
        m_triangleList[i].normal = {0, 0, 0};
        m_triangleList[i].centroid = {0, 0, 0};
    }
    // create the storage for the derived values
    m_vertexList.resize(m_markerList.size());
    m_pointForceList.resize(m_triangleList.size() * 3);

    std::vector<NamedObject *> upstreamObjects;
    upstreamObjects.reserve(m_markerList.size());
    for (auto &&it : m_markerList) upstreamObjects.push_back(it);
    setUpstreamObjects(std::move(upstreamObjects));
    return nullptr;
}

void FluidSac::saveToAttributes()
{
    this->setTag("FLUIDSAC"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

void FluidSac::appendToAttributes()
{
    NamedObject::appendToAttributes();
    std::string buf;
    setAttribute("NumMarkers"s, *GSUtil::ToString(m_markerList.size(), &buf));
    std::vector<std::string> stringList;
    stringList.reserve(m_markerList.size());
    for (size_t i = 0; i < m_markerList.size(); i++) stringList.push_back(m_markerList[i]->name());
    setAttribute("MarkerIDList"s, pystring::join(" "s, stringList));
    setAttribute("NumTriangles"s, *GSUtil::ToString(m_triangleList.size(), &buf));
    stringList.clear();
    stringList.reserve(m_triangleList.size());
    size_t triVertices[3];
    for (size_t i = 0; i < m_triangleList.size(); i++)
    {
        triVertices[0] = m_triangleList[i].v0; triVertices[1] = m_triangleList[i].v1; triVertices[2] = m_triangleList[i].v2;
        stringList.push_back(*GSUtil::ToString(triVertices, 3, &buf));
    }
    setAttribute("TriangleIndexList"s, pystring::join(" "s, stringList));
}

std::string FluidSac::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tVolume\tPressure\tNForces";
        for (size_t i = 0; i < m_pointForceList.size(); i++)
        {
            ss << "\tName" << i << "\tPX" << i << "\tPY" << i << "\tPZ" << i << "\tFX" <<
                  i << "\tFY" << i << "\tFZ" << i;
        }
        ss << "\n";
    }
    ss << simulation()->GetTime() << "\t" << m_sacVolume << "\t" << m_pressure << "\t" << m_pointForceList.size();
    for (size_t i = 0; i < m_pointForceList.size(); i++)
    {
        ss << "\t" << m_pointForceList[i].body->name() << "\t" << m_pointForceList[i].point[0] << "\t" << m_pointForceList[i].point[1] << "\t" << m_pointForceList[i].point[2] << "\t" <<
              m_pointForceList[i].vector[0] << "\t" << m_pointForceList[i].vector[1] << "\t" << m_pointForceList[i].vector[2];
    }
    ss << "\n";
    return ss.str();
}

void FluidSac::setSacVolume(double sacVolume)
{
    m_sacVolume = sacVolume;
//    double deltaTime = simulation()->GetTime() - m_lastTime;
//    if (deltaTime != 0) // FIX ME - this is not a good way of calculating the m_dotSacVolume. I should work from the marker velocities directly.
//    {
//        m_dotSacVolume = (m_sacVolume - m_lastSacVolume) / deltaTime;
//        m_lastSacVolume = m_sacVolume;
//        m_lastTime = simulation()->GetTime();
//    }
}

void FluidSac::setPressure(double pressure)
{
    m_pressure = pressure;
}

double FluidSac::dotSacVolume() const
{
    return m_dotSacVolume;
}

void FluidSac::setDotSacVolume(double newDotSacVolume)
{
    m_dotSacVolume = newDotSacVolume;
}

