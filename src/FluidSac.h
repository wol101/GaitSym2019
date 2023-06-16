/*
 *  FluidSac.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 02/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef FluidSac_H
#define FluidSac_H

#include "NamedObject.h"
#include "PGDMath.h"
#include "Strap.h"

#include <vector>
#include <tuple>

class Marker;

class FluidSac : public NamedObject
{
public:
    FluidSac();

    struct Triangle
    {
        size_t v0; size_t v1; size_t v2;
        double area;
        pgd::Vector3 normal;
        pgd::Vector3 centroid;
    };

    static double signedVolumeOfTriangle(pgd::Vector3 p1, pgd::Vector3 p2, pgd::Vector3 p3);
    static double volumeOfMesh(const std::vector<Triangle> &triangleList, const std::vector<pgd::Vector3> &vectorList);
    static double area(std::vector<std::pair<double, double>> points);
    static void areaCentroid(std::vector<std::pair<double, double>> points, double *area, std::pair<double, double> *centroid);
    static void areaCentroidNormal(const pgd::Vector3 &v0, const pgd::Vector3 &v1, const pgd::Vector3 &v2, double *area, pgd::Vector3 *centroid, pgd::Vector3 *normal);
    static void areaCentroidNormal(const pgd::Vector3 &v0, const pgd::Vector3 &v1, const pgd::Vector3 &v2, const pgd::Vector3 &v3, double *area, pgd::Vector3 *centroid, pgd::Vector3 *normal);
    static bool isGoodMesh(const std::vector<FluidSac::Triangle> &triangleList, const std::vector<Marker *> &markerList);

    virtual void calculateVolume();
    virtual void calculatePressure() = 0;
    virtual void calculateLoadsOnMarkers();
    virtual void LateInitialisation();

    double sacVolume() const;
    double dotSacVolume() const;
    double pressure() const;
    const std::vector<PointForce> &pointForceList() const;
    const std::vector<FluidSac::Triangle> &triangleList() const;
    size_t numTriangles() const;
    void triangleVertices(size_t triangleIndex, double vertices[9]) const;

    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();
    virtual std::string dumpToString();

    void setSacVolume(double sacVolume);
    void setPressure(double pressure);
    void setDotSacVolume(double newDotSacVolume);

private:
    std::vector<FluidSac::Triangle> m_triangleList;
    std::vector<Marker *> m_markerList;
    std::vector<pgd::Vector3> m_vertexList;
    std::vector<PointForce> m_pointForceList;

    double m_sacVolume = 0;
    double m_pressure = 0;
    // double m_lastSacVolume = 0;
    // double m_lastTime = 0;
    double m_dotSacVolume = 0;
};

#endif // FluidSac_H
