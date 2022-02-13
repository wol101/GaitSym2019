/*
 *  DrawMuscle.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "DrawMuscle.h"
#include "Muscle.h"
#include "Body.h"
#include "FacetedObject.h"
#include "Muscle.h"
#include "TwoPointStrap.h"
#include "NPointStrap.h"
#include "CylinderWrapStrap.h"
#include "FacetedConicSegment.h"
#include "FacetedPolyline.h"
#include "PGDMath.h"
#include "Preferences.h"
#include "DampedSpringMuscle.h"
#include "MAMuscle.h"
#include "MAMuscleComplete.h"
#include "GSUtil.h"
#include "Marker.h"
#include "TwoCylinderWrapStrap.h"

#include <QString>
#include <QDir>
#include <QDebug>

DrawMuscle::DrawMuscle()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    m_objectCountAtCreation = m_objectCount++;
    std::cerr << m_objectCountAtCreation << " " << className() << " constructed\n";;
#endif
    m_strapRadius = Preferences::valueDouble("StrapRadius");
    m_strapNumSegments = size_t(Preferences::valueInt("StrapSegments"));
    m_strapCylinderLength = Preferences::valueDouble("StrapCylinderLength");
    m_strapCylinderSegments = size_t(Preferences::valueDouble("StrapCylinderSegments"));
    m_strapCylinderWrapSegments = size_t(Preferences::valueInt("StrapCylinderWrapSegments"));
    m_displayMuscleForces = Preferences::valueBool("DisplayMuscleForces");
    m_strapForceScale = Preferences::valueDouble("StrapForceScale");
    m_strapForceRadius = Preferences::valueDouble("StrapForceRadius");
    m_strapForceColor = Preferences::valueQColor("StrapForceColour");
    m_strapColor = Preferences::valueQColor("StrapColour");
    m_strapCylinderColor = Preferences::valueQColor("StrapCylinderColour");
    m_strapColourMap = Colour::ColourMap(Preferences::valueInt("StrapColourMap"));
}

DrawMuscle::~DrawMuscle()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    std::cerr << m_objectCountAtCreation << " " << className() << " destructed\n";;
#endif
}

std::string DrawMuscle::name()
{
    if (m_muscle) return m_muscle->name();
    else return std::string();
}

Muscle *DrawMuscle::muscle() const
{
    return m_muscle;
}

double DrawMuscle::strapRadius() const
{
    return m_strapRadius;
}

void DrawMuscle::setStrapRadius(double strapRadius)
{
    m_strapRadius = strapRadius;
}

size_t DrawMuscle::strapNumSegments() const
{
    return m_strapNumSegments;
}

void DrawMuscle::setStrapNumSegments(size_t strapNumSegments)
{
    m_strapNumSegments = strapNumSegments;
}

double DrawMuscle::strapCylinderLength() const
{
    return m_strapCylinderLength;
}

void DrawMuscle::setStrapCylinderLength(double strapCylinderLength)
{
    m_strapCylinderLength = strapCylinderLength;
}

bool DrawMuscle::displayMuscleForces() const
{
    return m_displayMuscleForces;
}

void DrawMuscle::setDisplayMuscleForces(bool displayMuscleForces)
{
    m_displayMuscleForces = displayMuscleForces;
}

double DrawMuscle::strapForceScale() const
{
    return m_strapForceScale;
}

void DrawMuscle::setStrapForceScale(double strapForceScale)
{
    m_strapForceScale = strapForceScale;
}

double DrawMuscle::strapForceRadius() const
{
    return m_strapForceRadius;
}

void DrawMuscle::setStrapForceRadius(double strapForceRadius)
{
    m_strapForceRadius = strapForceRadius;
}

QColor DrawMuscle::strapForceColor() const
{
    return m_strapForceColor;
}

void DrawMuscle::setStrapForceColor(const QColor &strapForceColor)
{
    m_strapForceColor = strapForceColor;
}

QColor DrawMuscle::strapColor() const
{
    return m_strapColor;
}

void DrawMuscle::setStrapColor(const QColor &strapColor)
{
    m_strapColor = strapColor;
}

QColor DrawMuscle::strapCylinderColor() const
{
    return m_strapCylinderColor;
}

void DrawMuscle::setStrapCylinderColor(const QColor &strapCylinderColor)
{
    m_strapCylinderColor = strapCylinderColor;
}

Colour::ColourMap DrawMuscle::strapColourMap() const
{
    return m_strapColourMap;
}

void DrawMuscle::setStrapColourMap(const Colour::ColourMap &strapColourMap)
{
    m_strapColourMap = strapColourMap;
}

void DrawMuscle::setMuscle(Muscle *muscle)
{
    m_muscle = muscle;
}

void DrawMuscle::initialise(SimulationWidget *simulationWidget)
{
    if (!m_muscle) return;

    Colour colour(m_muscle->GetStrap()->colour1());
    switch (m_muscle->strapColourControl())
    {
    case Muscle::fixedColour:
        m_strapColor.setRedF(qreal(m_muscle->GetStrap()->colour1().r()));
        m_strapColor.setGreenF(qreal(m_muscle->GetStrap()->colour1().g()));
        m_strapColor.setBlueF(qreal(m_muscle->GetStrap()->colour1().b()));
        m_strapColor.setAlphaF(qreal(m_muscle->GetStrap()->colour1().alpha()));
        break;
    case Muscle::activationMap:
        Colour::SetColourFromMap(float(m_muscle->GetActivation()), m_strapColourMap, &colour, false);
        m_strapColor = QColor(QString::fromStdString(colour.GetHexArgb()));
        break;
    case Muscle::strainMap:
        if (dynamic_cast<DampedSpringMuscle *>(m_muscle)) Colour::SetColourFromMap(float(m_muscle->GetLength() / dynamic_cast<DampedSpringMuscle *>(m_muscle)->GetUnloadedLength()) - 0.5f, m_strapColourMap, &colour, false);
        else if (dynamic_cast<MAMuscleComplete *>(m_muscle)) Colour::SetColourFromMap(
                    float(m_muscle->GetLength() / (dynamic_cast<MAMuscleComplete *>(m_muscle)->fibreLength() + dynamic_cast<MAMuscleComplete *>(m_muscle)->tendonLength())) - 0.5f, m_strapColourMap, &colour, false);
        else if (dynamic_cast<MAMuscle *>(m_muscle)) Colour::SetColourFromMap(float(m_muscle->GetLength() / (dynamic_cast<MAMuscle *>(m_muscle)->fibreLength())) - 0.5f, m_strapColourMap, &colour, false);
        m_strapColor = QColor(QString::fromStdString(colour.GetHexArgb()));
        break;
    case Muscle::forceMap:
        if (dynamic_cast<DampedSpringMuscle *>(m_muscle)) Colour::SetColourFromMap(
                    float(m_muscle->GetTension() / (dynamic_cast<DampedSpringMuscle *>(m_muscle)->GetUnloadedLength() * dynamic_cast<DampedSpringMuscle *>(m_muscle)->GetSpringConstant())), m_strapColourMap, &colour, false);
        else if (dynamic_cast<MAMuscleComplete *>(m_muscle)) Colour::SetColourFromMap(
                    float(m_muscle->GetTension() / (dynamic_cast<MAMuscleComplete *>(m_muscle)->forcePerUnitArea() * dynamic_cast<MAMuscleComplete *>(m_muscle)->pca())), m_strapColourMap, &colour, false);
        else if (dynamic_cast<MAMuscle *>(m_muscle)) Colour::SetColourFromMap(
                    float(m_muscle->GetLength() / (dynamic_cast<MAMuscle *>(m_muscle)->forcePerUnitArea() * dynamic_cast<MAMuscle *>(m_muscle)->pca())), m_strapColourMap, &colour, false);
        m_strapColor = QColor(QString::fromStdString(colour.GetHexArgb()));
        break;
    }
    m_strapCylinderColor.setRedF(qreal(m_muscle->GetStrap()->colour2().r()));
    m_strapCylinderColor.setGreenF(qreal(m_muscle->GetStrap()->colour2().g()));
    m_strapCylinderColor.setBlueF(qreal(m_muscle->GetStrap()->colour2().b()));
    m_strapCylinderColor.setAlphaF(qreal(m_muscle->GetStrap()->colour2().alpha()));

    m_strapForceColor.setRedF(qreal(m_muscle->colour1().r()));
    m_strapForceColor.setGreenF(qreal(m_muscle->colour1().g()));
    m_strapForceColor.setBlueF(qreal(m_muscle->colour1().b()));
    m_strapForceColor.setAlphaF(qreal(m_muscle->colour1().alpha()));

    m_strapRadius = m_muscle->GetStrap()->size1();
    m_strapCylinderLength  = m_muscle->GetStrap()->size2();
    m_strapForceRadius  = m_muscle->size1();
    m_strapForceScale  = m_muscle->size2();

    for (bool first = true; first; first = false) // this loop runs once to avoid nasty nested if-else statements
    {
        TwoPointStrap *twoPointStrap = dynamic_cast<TwoPointStrap *>(m_muscle->GetStrap());
        if (twoPointStrap)
        {
            std::vector<std::unique_ptr<PointForce >> *pointForceList = twoPointStrap->GetPointForceList();
            std::vector<pgd::Vector3> polyline;
            polyline.reserve(pointForceList->size());
            polyline.push_back(pgd::Vector3(pointForceList->at(0)->point[0], pointForceList->at(0)->point[1], pointForceList->at(0)->point[2]));
            polyline.push_back(pgd::Vector3(pointForceList->at(1)->point[0], pointForceList->at(1)->point[1], pointForceList->at(1)->point[2]));
            m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyline, m_strapRadius, m_strapNumSegments, m_strapColor, 1);
            m_facetedObject1->setSimulationWidget(simulationWidget);
            m_facetedObjectList.push_back(m_facetedObject1.get());
            break;
        }
        NPointStrap *nPointStrap = dynamic_cast<NPointStrap *>(m_muscle->GetStrap());
        if (nPointStrap)
        {
            std::vector<std::unique_ptr<PointForce >> *pointForceList = nPointStrap->GetPointForceList();
            std::vector<pgd::Vector3> polyline;
            polyline.reserve(pointForceList->size());
            polyline.push_back(pgd::Vector3(pointForceList->at(0)->point[0], pointForceList->at(0)->point[1], pointForceList->at(0)->point[2]));
            for (size_t i = 2; i < pointForceList->size(); i++)
                polyline.push_back(pgd::Vector3(pointForceList->at(i)->point[0], pointForceList->at(i)->point[1], pointForceList->at(i)->point[2]));
            polyline.push_back(pgd::Vector3(pointForceList->at(1)->point[0], pointForceList->at(1)->point[1], pointForceList->at(1)->point[2]));
            m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyline, m_strapRadius, m_strapNumSegments, m_strapColor, 1);
            m_facetedObject1->setSimulationWidget(simulationWidget);
            m_facetedObjectList.push_back(m_facetedObject1.get());
            break;
        }
        CylinderWrapStrap *cylinderWrapStrap = dynamic_cast<CylinderWrapStrap *>(m_muscle->GetStrap());
        if (cylinderWrapStrap)
        {
            if (cylinderWrapStrap->GetNumWrapSegments() != int(m_strapCylinderWrapSegments))
            {
                cylinderWrapStrap->SetNumWrapSegments(int(m_strapCylinderWrapSegments));
                cylinderWrapStrap->Calculate();
            }
            std::vector<pgd::Vector3> polyline = *cylinderWrapStrap->GetPathCoordinates();
            if (polyline.size())
            {
                m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyline, m_strapRadius, m_strapNumSegments, m_strapColor, 1);
                m_facetedObject1->setSimulationWidget(simulationWidget);
                m_facetedObjectList.push_back(m_facetedObject1.get());
            }

            // calculate the quaternion that rotates from cylinder coordinates to world coordinates
//            const Body *body;
//            dVector3 pos;
//            dQuaternion qq;
//            double radius;
//            cylinderWrapStrap->GetCylinder(&body, pos, &radius, qq);
//            const double *q = dBodyGetQuaternion(body->GetBodyID());
//            pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
//            pgd::Quaternion cylinderToWorldQuaternion =  qBody * pgd::Quaternion(qq[0], qq[1], qq[2], qq[3]);
//            pgd::Vector3 cylinderVecWorld = pgd::QVRotate(cylinderToWorldQuaternion, pgd::Vector3(0, 0, m_strapCylinderLength / 2));
//            // calculate the cylinder world position
//            dVector3 position;
//            dBodyGetRelPointPos(body->GetBodyID(), pos[0], pos[1], pos[2], position);
            pgd::Vector3 position = cylinderWrapStrap->GetCylinderMarker()->GetWorldPosition();
//            pgd::Vector3 cylinderVecWorld = pgd::QVRotate(cylinderWrapStrap->GetCylinderMarker()->GetWorldQuaternion(), pgd::Vector3(0, 0, m_strapCylinderLength / 2));
            pgd::Vector3 cylinderVecWorld = pgd::QVRotate(cylinderWrapStrap->GetCylinderMarker()->GetWorldQuaternion(), pgd::Vector3(m_strapCylinderLength / 2, 0, 0));
            double radius = cylinderWrapStrap->cylinderRadius();
            // and draw it
            polyline.clear();
            polyline.push_back(pgd::Vector3(position[0] - cylinderVecWorld.x, position[1] - cylinderVecWorld.y, position[2] - cylinderVecWorld.z));
            polyline.push_back(pgd::Vector3(position[0] + cylinderVecWorld.x, position[1] + cylinderVecWorld.y, position[2] + cylinderVecWorld.z));
            m_facetedObject2 = std::make_unique<FacetedPolyline>(&polyline, radius, m_strapCylinderSegments, m_strapCylinderColor, 1);
            m_facetedObject2->setSimulationWidget(simulationWidget);
            m_facetedObjectList.push_back(m_facetedObject2.get());
            break;
        }
        TwoCylinderWrapStrap *twoCylinderWrapStrap = dynamic_cast<TwoCylinderWrapStrap *>(m_muscle->GetStrap());
        if (twoCylinderWrapStrap)
        {
            if (twoCylinderWrapStrap->GetNumWrapSegments() != int(m_strapCylinderWrapSegments))
            {
                twoCylinderWrapStrap->SetNumWrapSegments(int(m_strapCylinderWrapSegments));
                twoCylinderWrapStrap->Calculate();
            }
            std::vector<pgd::Vector3> polyline = *twoCylinderWrapStrap->GetPathCoordinates();
            if (polyline.size())
            {
                m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyline, m_strapRadius, m_strapNumSegments, m_strapColor, 1);
                m_facetedObject1->setSimulationWidget(simulationWidget);
                m_facetedObjectList.push_back(m_facetedObject1.get());
            }
            pgd::Vector3 position = twoCylinderWrapStrap->GetCylinder1Marker()->GetWorldPosition();
            pgd::Vector3 cylinderVecWorld = pgd::QVRotate(twoCylinderWrapStrap->GetCylinder1Marker()->GetWorldQuaternion(), pgd::Vector3(m_strapCylinderLength / 2, 0, 0));
            double radius = twoCylinderWrapStrap->Cylinder1Radius();
            // and draw it
            polyline.clear();
            polyline.push_back(pgd::Vector3(position[0] - cylinderVecWorld.x, position[1] - cylinderVecWorld.y, position[2] - cylinderVecWorld.z));
            polyline.push_back(pgd::Vector3(position[0] + cylinderVecWorld.x, position[1] + cylinderVecWorld.y, position[2] + cylinderVecWorld.z));
            m_facetedObject2 = std::make_unique<FacetedPolyline>(&polyline, radius, m_strapCylinderSegments, m_strapCylinderColor, 1);
            m_facetedObject2->setSimulationWidget(simulationWidget);
            m_facetedObjectList.push_back(m_facetedObject2.get());
            position = twoCylinderWrapStrap->GetCylinder2Marker()->GetWorldPosition();
            radius = twoCylinderWrapStrap->Cylinder2Radius();
            // and draw it
            polyline.clear();
            polyline.push_back(pgd::Vector3(position[0] - cylinderVecWorld.x, position[1] - cylinderVecWorld.y, position[2] - cylinderVecWorld.z));
            polyline.push_back(pgd::Vector3(position[0] + cylinderVecWorld.x, position[1] + cylinderVecWorld.y, position[2] + cylinderVecWorld.z));
            m_facetedObject3 = std::make_unique<FacetedPolyline>(&polyline, radius, m_strapCylinderSegments, m_strapCylinderColor, 1);
            m_facetedObject3->setSimulationWidget(simulationWidget);
            m_facetedObjectList.push_back(m_facetedObject3.get());
            break;
        }
        qDebug() << "Error in DrawMuscle::initialise: Unsupported STRAP type";
    }

    if (m_displayMuscleForces)
    {
        std::vector<std::unique_ptr<PointForce >> *pointForceList = m_muscle->GetPointForceList();
        for (size_t i = 0; i < pointForceList->size(); i++)
        {
            std::vector<pgd::Vector3> polyline;
            polyline.reserve(2);
            polyline.clear();
            pgd::Vector3 f = pgd::Vector3(pointForceList->at(i)->vector[0], pointForceList->at(i)->vector[1], pointForceList->at(i)->vector[2]) * m_muscle->GetTension() * m_strapForceScale;
            polyline.push_back(pgd::Vector3(pointForceList->at(i)->point[0], pointForceList->at(i)->point[1], pointForceList->at(i)->point[2]));
            polyline.push_back(pgd::Vector3(pointForceList->at(i)->point[0], pointForceList->at(i)->point[1], pointForceList->at(i)->point[2]) + f);
            std::unique_ptr<FacetedPolyline>facetedPolyline = std::make_unique<FacetedPolyline>(&polyline, m_strapForceRadius, m_strapNumSegments, m_strapForceColor, 1);
            facetedPolyline->setSimulationWidget(simulationWidget);
            m_facetedObjectForceList.push_back(std::move(facetedPolyline));
            m_facetedObjectList.push_back(facetedPolyline.get());
        }
    }

//    qDebug() << "DrawMuscle::initialise: finished " << m_muscle->name().c_str();

    return;
}

void DrawMuscle::Draw()
{
    if (m_facetedObject1.get()) m_facetedObject1->Draw();
    if (m_facetedObject2.get()) m_facetedObject2->Draw();
    if (m_facetedObject3.get()) m_facetedObject3->Draw();
    for (size_t i = 0; i < m_facetedObjectForceList.size(); i++)
    {
        m_facetedObjectForceList.at(i)->Draw();
    }
    m_muscle->setRedraw(false);
}


