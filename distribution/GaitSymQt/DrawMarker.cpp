/*
 *  DrawMarker.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 11/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "DrawMarker.h"
#include "Marker.h"
#include "DrawAxes.h"
#include "FacetedObject.h"
#include "PGDMath.h"
#include "Preferences.h"
#include "FacetedSphere.h"

#include <QString>
#include <QTransform>
#include <QDiffuseSpecularMaterial>

DrawMarker::DrawMarker(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
    m_marker = nullptr;
    m_markerTransform = nullptr;
    m_radius = Preferences::valueDouble("MarkerRadius");
    m_level = Preferences::valueInt("MarkerSphereLevel");
    m_color = Preferences::valueQColor("MarkerColour");
}

Marker *DrawMarker::marker() const
{
    return m_marker;
}

double DrawMarker::radius() const
{
    return m_radius;
}

void DrawMarker::setRadius(double radius)
{
    m_radius = radius;
}

int DrawMarker::level() const
{
    return m_level;
}

void DrawMarker::setLevel(int level)
{
    m_level = level;
}

QColor DrawMarker::color() const
{
    return m_color;
}

void DrawMarker::setColor(const QColor &color)
{
    m_color = color;
}

Qt3DRender::QLayer *DrawMarker::layer() const
{
    return m_layer;
}

void DrawMarker::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

SceneEffect *DrawMarker::effect() const
{
    return m_effect;
}

void DrawMarker::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}

int DrawMarker::initialise(Marker *marker)
{
    if (!marker) return __LINE__;

    m_marker = marker;
    m_color.setRedF(qreal(marker->colour1().r()));
    m_color.setGreenF(qreal(marker->colour1().g()));
    m_color.setBlueF(qreal(marker->colour1().b()));
    m_color.setAlphaF(qreal(marker->colour1().alpha()));
    m_radius = marker->size1();

    m_markerTransform = new Qt3DCore::QTransform(this);
    this->addComponent(m_markerTransform);

    FacetedSphere *meshEntity = new FacetedSphere(m_radius, m_level, m_color, this);
    meshEntity->setLayer(m_layer);
    meshEntity->setEffect(m_effect);
    meshEntity->InitialiseEntity();
    pgd::Vector p = m_marker->GetWorldPosition();
    pgd::Quaternion q = m_marker->GetWorldQuaternion();
    m_markerTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
    m_markerTransform->setTranslation(QVector3D(float(p.x), float(p.y), float(p.z)));

    return 0;
}

void DrawMarker::updateEntityPose()
{
    pgd::Vector p = m_marker->GetWorldPosition();
    pgd::Quaternion q = m_marker->GetWorldQuaternion();
    m_markerTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
    m_markerTransform->setTranslation(QVector3D(float(p.x), float(p.y), float(p.z)));
}


