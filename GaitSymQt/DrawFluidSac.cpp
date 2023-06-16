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

DrawFluidSac::DrawFluidSac()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    m_objectCountAtCreation = m_objectCount++;
    std::cerr << m_objectCountAtCreation << " " << className() << " constructed\n";;
#endif
    m_fluidSacColour = Preferences::valueQColor("FluidSacColour");
    m_fluidSacForceColour = Preferences::valueQColor("FluidSacForceColour");
    m_displayFluidSacForces = Preferences::valueBool("DisplayFluidSacForces");
    m_fluidSacForceScale = Preferences::valueDouble("FluidSacForceScale");
    m_fluidSacForceRadius = Preferences::valueDouble("FluidSacForceRadius");
    m_fluidSacForceSegments = size_t(Preferences::valueInt("FluidSacForceSegments"));
}

DrawFluidSac::~DrawFluidSac()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    std::cerr << m_objectCountAtCreation << " " << className() << " destructed\n";;
#endif
}

std::string DrawFluidSac::name()
{
    if (m_fluidSac) return m_fluidSac->name();
    else return std::string();
}

void DrawFluidSac::initialise(SimulationWidget *simulationWidget)
{
    if (!m_fluidSac) return;

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

    m_facetedObject = std::make_unique<FacetedObject>();
    m_facetedObject->setSimulationWidget(simulationWidget);
    m_facetedObject->setBlendColour(m_fluidSacColour, 1);
    m_facetedObject->AllocateMemory(m_fluidSac->numTriangles());
    double vertices[9];
    for (size_t i = 0; i < m_fluidSac->numTriangles(); i++)
    {
        m_fluidSac->triangleVertices(i, vertices);
        m_facetedObject->AddTriangle(vertices);
    }
//    qDebug() << "DrawFluidSac " << facetedObject->GetNumTriangles() << " triangles created\n";

    if (m_displayFluidSacForces)
    {
        const std::vector<PointForce> &pointForceList = m_fluidSac->pointForceList();
        for (size_t i = 0; i < pointForceList.size(); i++)
        {
            std::vector<pgd::Vector3> polyline;
            polyline.clear();
            pgd::Vector3 f = pgd::Vector3(pointForceList.at(i).vector[0], pointForceList.at(i).vector[1], pointForceList.at(i).vector[2]) * m_fluidSacForceScale;
            polyline.push_back(pgd::Vector3(pointForceList.at(i).point[0], pointForceList.at(i).point[1], pointForceList.at(i).point[2]));
            polyline.push_back(pgd::Vector3(pointForceList.at(i).point[0], pointForceList.at(i).point[1], pointForceList.at(i).point[2]) + f);
            std::unique_ptr<FacetedObject> facetedPolyline = std::make_unique<FacetedPolyline>(&polyline, m_fluidSacForceRadius, m_fluidSacForceSegments, m_fluidSacForceColour, 1);
            m_facetedObjectForceList.push_back(std::move(facetedPolyline));
        }
    }
    m_facetedObjectList.push_back(m_facetedObject.get());
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

void DrawFluidSac::setFluidSac(FluidSac *fluidSac)
{
    m_fluidSac = fluidSac;
}

void DrawFluidSac::Draw()
{
    m_facetedObject->Draw();
    for (size_t i = 0; i < m_facetedObjectForceList.size(); i++)
    {
        m_facetedObjectForceList.at(i)->Draw();
    }
    m_fluidSac->setRedraw(false);
}


