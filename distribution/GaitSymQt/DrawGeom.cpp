/*
 *  DrawGeom.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 11/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "DrawGeom.h"
#include "Geom.h"
#include "FacetedObject.h"
#include "FacetedSphere.h"
#include "FacetedBox.h"
#include "FacetedCappedCylinder.h"
#include "FacetedRect.h"
#include "Marker.h"
#include "CappedCylinderGeom.h"
#include "SphereGeom.h"
#include "BoxGeom.h"
#include "PlaneGeom.h"
#include "FacetedConicSegment.h"
#include "FacetedPolyline.h"
#include "FacetedCheckerboard.h"
#include "PGDMath.h"
#include "Preferences.h"

#include <QString>
#include <QDir>
#include <QDebug>

#include <vector>

DrawGeom::DrawGeom()
{
    m_geomColor1 = Preferences::valueQColor("GeomColour1");
    m_geomColor2 = Preferences::valueQColor("GeomColour2");
    m_geomSegments = size_t(Preferences::valueInt("GeomSegments"));
    m_geomSize1 = Preferences::valueDouble("GeomSize1");
    m_geomSize2 = Preferences::valueDouble("GeomSize2");
}

std::string DrawGeom::name()
{
    if (m_geom) return m_geom->name();
    else return std::string();
}

Geom *DrawGeom::geom() const
{
    return m_geom;
}

void DrawGeom::setGeom(Geom *geom)
{
    m_geom = geom;
}

void DrawGeom::initialise(SimulationWidget *simulationWidget)
{
    if (!m_geom) return;

    m_geomColor1.setRedF(qreal(m_geom->colour1().r()));
    m_geomColor1.setGreenF(qreal(m_geom->colour1().g()));
    m_geomColor1.setBlueF(qreal(m_geom->colour1().b()));
    m_geomColor1.setAlphaF(qreal(m_geom->colour1().alpha()));
    m_geomColor2.setRedF(qreal(m_geom->colour2().r()));
    m_geomColor2.setGreenF(qreal(m_geom->colour2().g()));
    m_geomColor2.setBlueF(qreal(m_geom->colour2().b()));
    m_geomColor2.setAlphaF(qreal(m_geom->colour2().alpha()));
    m_geomSize1 = m_geom->size1();
    m_geomSize2 = m_geom->size2();

    SphereGeom *sphereGeom = dynamic_cast<SphereGeom *>(m_geom);
    if (sphereGeom)
    {
        m_facetedObject = std::make_unique<FacetedSphere>(sphereGeom->radius(), FacetedSphere::EstimateLevel(m_geomSegments), m_geomColor1, 1);
        m_facetedObject->setSimulationWidget(simulationWidget);
        m_facetedObjectList.push_back(m_facetedObject.get());
        return;
    }

    CappedCylinderGeom *cappedCylinderGeom = dynamic_cast<CappedCylinderGeom *>(m_geom);
    if (cappedCylinderGeom)
    {
        double length, radius;
        cappedCylinderGeom->getLengthRadius(&length, &radius);
        m_facetedObject = std::make_unique<FacetedCappedCylinder>(length, radius, m_geomSegments / 4, m_geomColor1, 1);
        m_facetedObject->setSimulationWidget(simulationWidget);
        m_facetedObjectList.push_back(m_facetedObject.get());
        return;
    }

    PlaneGeom *planeGeom = dynamic_cast<PlaneGeom *>(m_geom);
    if (planeGeom)
    {
        double planeSize = m_geomSize1;
        double checkerSize = m_geomSize2;
        if (planeSize < 2 * checkerSize) planeSize = 2 * checkerSize;
        size_t nx = size_t(planeSize / checkerSize);
        size_t ny = nx;
        m_facetedObject = std::make_unique<FacetedCheckerboard>(nx, ny, checkerSize, checkerSize, m_geomColor1, m_geomColor2);
        m_facetedObject->setSimulationWidget(simulationWidget);
        Marker *marker = planeGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector p = marker->GetWorldPosition();
        SetDisplayRotation(q.data());
        SetDisplayPosition(p.x, p.y, p.z);
        m_facetedObjectList.push_back(m_facetedObject.get());
        return;
    }

    qDebug() << "Error in DrawGeom::initialise: Unsupported GEOM type";
}

void DrawGeom::updateEntityPose()
{
    SphereGeom *sphereGeom = dynamic_cast<SphereGeom *>(m_geom);
    if (sphereGeom)
    {
        Marker *marker = sphereGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector p = marker->GetWorldPosition();
        SetDisplayRotationFromQuaternion(q.data());
        SetDisplayPosition(p.x, p.y, p.z);
        return;
    }

    CappedCylinderGeom *cappedCylinderGeom = dynamic_cast<CappedCylinderGeom *>(m_geom);
    if (cappedCylinderGeom)
    {
        Marker *marker = cappedCylinderGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector p = marker->GetWorldPosition();
        SetDisplayRotationFromQuaternion(q.data());
        SetDisplayPosition(p.x, p.y, p.z);
        return;
    }
}

void DrawGeom::Draw()
{
    m_facetedObject->Draw();
}

