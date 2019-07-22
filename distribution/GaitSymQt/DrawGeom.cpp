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
#include "DrawAxes.h"
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
#include <QMesh>
#include <QTransform>
#include <QDiffuseSpecularMaterial>

#include <vector>

DrawGeom::DrawGeom(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
    m_geomColor1 = Preferences::valueQColor("GeomColour1");
    m_geomColor2 = Preferences::valueQColor("GeomColour2");
    m_geomSegments = Preferences::valueInt("GeomSegments");
}

Geom *DrawGeom::geom() const
{
    return m_geom;
}

Qt3DRender::QLayer *DrawGeom::layer() const
{
    return m_layer;
}

void DrawGeom::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

SceneEffect *DrawGeom::effect() const
{
    return m_effect;
}

void DrawGeom::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}

int DrawGeom::initialise(Geom *geom)
{
    if (!geom) return __LINE__;

    m_geom = geom;
    m_geomColor1.setRedF(qreal(m_geom->colour1().r()));
    m_geomColor1.setGreenF(qreal(m_geom->colour1().g()));
    m_geomColor1.setBlueF(qreal(m_geom->colour1().b()));
    m_geomColor1.setAlphaF(qreal(m_geom->colour1().alpha()));
    m_geomColor2.setRedF(qreal(m_geom->colour2().r()));
    m_geomColor2.setGreenF(qreal(m_geom->colour2().g()));
    m_geomColor2.setBlueF(qreal(m_geom->colour2().b()));
    m_geomColor2.setAlphaF(qreal(m_geom->colour2().alpha()));

    m_geomTransform = new Qt3DCore::QTransform(this);
    this->addComponent(m_geomTransform);

    SphereGeom *sphereGeom = dynamic_cast<SphereGeom *>(m_geom);
    if (sphereGeom)
    {
        FacetedSphere *meshEntity1 = new FacetedSphere(sphereGeom->radius(),
                FacetedSphere::EstimateLevel(m_geomSegments), m_geomColor1, this);
        meshEntity1->setLayer(m_layer);
        meshEntity1->setEffect(m_effect);
        meshEntity1->InitialiseEntity();

        Marker *marker = sphereGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_geomTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_geomTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return 0;
    }

    CappedCylinderGeom *cappedCylinderGeom = dynamic_cast<CappedCylinderGeom *>(m_geom);
    if (cappedCylinderGeom)
    {
        double length, radius;
        cappedCylinderGeom->getLengthRadius(&length, &radius);
        FacetedCappedCylinder *meshEntity1 = new FacetedCappedCylinder(length, radius, m_geomSegments / 4,
                m_geomColor1, this);
        meshEntity1->setLayer(m_layer);
        meshEntity1->setEffect(m_effect);
        meshEntity1->InitialiseEntity();

        Marker *marker = cappedCylinderGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_geomTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_geomTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return 0;
    }

    PlaneGeom *planeGeom = dynamic_cast<PlaneGeom *>(m_geom);
    if (planeGeom)
    {
        double planeSize = planeGeom->size1();
        double checkerSize = planeGeom->size2();
        int nx = int(planeSize / checkerSize);
        int ny = nx;
        FacetedCheckerboard *meshEntity1 = new FacetedCheckerboard(nx, ny, checkerSize, checkerSize,
                m_geomColor1, m_geomColor2, this);
        meshEntity1->setLayer(m_layer);
        meshEntity1->setEffect(m_effect);
        meshEntity1->InitialiseEntity();

        Marker *marker = planeGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_geomTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_geomTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return 0;
    }

    return 0;
}

void DrawGeom::updateEntityPose()
{
    SphereGeom *sphereGeom = dynamic_cast<SphereGeom *>(m_geom);
    if (sphereGeom)
    {
        Marker *marker = sphereGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_geomTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_geomTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return;
    }

    CappedCylinderGeom *cappedCylinderGeom = dynamic_cast<CappedCylinderGeom *>(m_geom);
    if (cappedCylinderGeom)
    {
        Marker *marker = cappedCylinderGeom->geomMarker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_geomTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_geomTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return;
    }
}

