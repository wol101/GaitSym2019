/*
 *  ConvexGeom.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 27/02/2021.
 *  Copyright 2021 Bill Sellers. All rights reserved.
 *
 */

#ifndef CONVEXGEOM_H
#define CONVEXGEOM_H

#include "Geom.h"

class ConvexGeom: public Geom
{
public:
    ConvexGeom(dSpaceID space, const double *planes, unsigned int planecount, const double *points, unsigned int pointcount, const unsigned int *polygons);

    void setConvex(const double *planes, unsigned int planecount, const double *points, unsigned int pointcount, const unsigned int *polygons);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    std::vector<double> *vertices();

    std::vector<int> *triangles();

private:
    void initialiseConvexData();
    // these are the data as supplied in the XML
    std::vector<double> m_vertices;
    std::vector<int> m_triangles;
    bool m_reverseWinding = false;
    // these are the data as required by the setConvex routine
    std::vector<double> m_planes;
    unsigned int m_planecount = 0;
    std::vector <double> m_points;
    unsigned int m_pointcount = 0;
    std::vector<unsigned int> m_polygons;
    int m_indexStart = 0; // allows both 1 indexed and 0 indexed triangle to make pasting in OBJ data easier
};

#endif // CONVEXGEOM_H
