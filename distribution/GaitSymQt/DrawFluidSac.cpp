/*
 *  DrawFluidSac.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "DrawFluidSac.h"
#include "FluidSac.h"
#include "FacetedObject.h"
#include "PGDMath.h"
#include "Preferences.h"
#include "FacetedPolyline.h"

#include <QString>
#include <QDir>
#include <QMesh>
#include <QTransform>
#include <QDiffuseSpecularMaterial>

DrawFluidSac::DrawFluidSac(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
    m_fluidSacColour = Preferences::valueQColor("FluidSacColour");
    m_fluidSacForceColour = Preferences::valueQColor("FluidSacForceColour");
    m_displayFluidSacForces = Preferences::valueBool("DisplayFluidSacForces");
    m_fluidSacForceScale = Preferences::valueDouble("FluidSacForceScale");
    m_fluidSacForceRadius = Preferences::valueDouble("FluidSacForceRadius");
    m_fluidSacForceSegments = Preferences::valueInt("FluidSacForceSegments");
}

int DrawFluidSac::initialise(FluidSac *fluidSac)
{
    if (!fluidSac) return __LINE__;

    m_fluidSac = fluidSac;
    m_fluidSacTransform = new Qt3DCore::QTransform(this);
    this->addComponent(m_fluidSacTransform);
    m_fluidSacColour.setRedF(qreal(m_fluidSac->colour1().r()));
    m_fluidSacColour.setGreenF(qreal(m_fluidSac->colour1().g()));
    m_fluidSacColour.setBlueF(qreal(m_fluidSac->colour1().b()));
    m_fluidSacColour.setAlphaF(qreal(m_fluidSac->colour1().alpha()));
    m_fluidSacForceColour.setRedF(qreal(m_fluidSac->colour2().r()));
    m_fluidSacForceColour.setGreenF(qreal(m_fluidSac->colour2().g()));
    m_fluidSacForceColour.setBlueF(qreal(m_fluidSac->colour2().b()));
    m_fluidSacForceColour.setAlphaF(qreal(m_fluidSac->colour2().alpha()));

    m_fluidSacForceRadius  = m_fluidSac->size1();
    m_fluidSacForceScale  = m_fluidSac->size2();

    FacetedObject *facetedObject = new FacetedObject(this);
    facetedObject->setLayer(m_layer);
    facetedObject->setEffect(m_effect);
    facetedObject->SetColour(m_fluidSacColour);
    facetedObject->AllocateMemory(m_fluidSac->numTriangles());
    double vertices[9];
    for (size_t i = 0; i < m_fluidSac->numTriangles(); i++)
    {
        m_fluidSac->triangleVertices(i, vertices);
        facetedObject->AddTriangle(vertices);
    }
    facetedObject->InitialiseEntity();
//    qDebug() << "DrawFluidSac " << facetedObject->GetNumTriangles() << " triangles created\n";

    if (m_displayFluidSacForces)
    {
        const std::vector<PointForce> &pointForceList = m_fluidSac->pointForceList();
        for (size_t i = 0; i < pointForceList.size(); i++)
        {
            std::vector<pgd::Vector> polyline;
            polyline.clear();
            pgd::Vector f = pgd::Vector(pointForceList.at(i).vector[0], pointForceList.at(i).vector[1],
                                        pointForceList.at(i).vector[2]) * m_fluidSacForceScale;
            polyline.push_back(pgd::Vector(pointForceList.at(i).point[0], pointForceList.at(i).point[1],
                                           pointForceList.at(i).point[2]));
            polyline.push_back(pgd::Vector(pointForceList.at(i).point[0], pointForceList.at(i).point[1],
                                           pointForceList.at(i).point[2]) + f);
            FacetedPolyline *facetedPolyline = new FacetedPolyline(&polyline, m_fluidSacForceRadius,
                    m_fluidSacForceSegments, m_fluidSacForceColour, this);
            facetedPolyline->setLayer(m_layer);
            facetedPolyline->setEffect(m_effect);
            facetedPolyline->InitialiseEntity();
        }
    }

    return 0;
}

FluidSac *DrawFluidSac::fluidSac() const
{
    return m_fluidSac;
}

QColor DrawFluidSac::fluidSacColour() const
{
    return m_fluidSacColour;
}

void DrawFluidSac::setFluidSacColour(const QColor &fluidSacColour)
{
    m_fluidSacColour = fluidSacColour;
}

bool DrawFluidSac::displayFluidSacForces() const
{
    return m_displayFluidSacForces;
}

void DrawFluidSac::setDisplayFluidSacForces(bool displayFluidSacForces)
{
    m_displayFluidSacForces = displayFluidSacForces;
}

double DrawFluidSac::fluidSacForceScale() const
{
    return m_fluidSacForceScale;
}

void DrawFluidSac::setFluidSacForceScale(double fluidSacForceScale)
{
    m_fluidSacForceScale = fluidSacForceScale;
}

Qt3DRender::QLayer *DrawFluidSac::layer() const
{
    return m_layer;
}

void DrawFluidSac::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

SceneEffect *DrawFluidSac::effect() const
{
    return m_effect;
}

void DrawFluidSac::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}



