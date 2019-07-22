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
#include "DrawAxes.h"
#include "FacetedObject.h"
#include "Muscle.h"
#include "TwoPointStrap.h"
#include "CylinderWrapStrap.h"
#include "FacetedConicSegment.h"
#include "FacetedPolyline.h"
#include "PGDMath.h"
#include "Preferences.h"

#include <QString>
#include <QDir>
#include <QMesh>
#include <QTransform>
#include <QDiffuseSpecularMaterial>

DrawMuscle::DrawMuscle(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
    m_muscle = nullptr;
    m_muscleTransform = nullptr;
    m_strapRadius = Preferences::valueDouble("StrapRadius");
    m_strapNumSegments = Preferences::valueInt("StrapSegments");
    m_strapCylinderLength = Preferences::valueDouble("StrapCylinderLength");
    m_displayMuscleForces = Preferences::valueBool("DisplayMuscleForces");
    m_strapForceScale = Preferences::valueDouble("StrapForceScale");
    m_strapForceRadius = Preferences::valueDouble("StrapForceRadius");
    m_strapForceColor = Preferences::valueQColor("StrapForceColour");
    m_strapColor = Preferences::valueQColor("StrapColour");
    m_strapCylinderColor = Preferences::valueQColor("StrapCylinderColour");
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

int DrawMuscle::strapNumSegments() const
{
    return m_strapNumSegments;
}

void DrawMuscle::setStrapNumSegments(int strapNumSegments)
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

Qt3DRender::QLayer *DrawMuscle::layer() const
{
    return m_layer;
}

void DrawMuscle::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

SceneEffect *DrawMuscle::effect() const
{
    return m_effect;
}

void DrawMuscle::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}

int DrawMuscle::initialise(Muscle *muscle)
{
    if (!muscle) return __LINE__;

    m_muscle = muscle;

    m_muscleTransform = new Qt3DCore::QTransform(this);
    this->addComponent(m_muscleTransform);
    m_strapColor.setRedF(qreal(m_muscle->GetStrap()->colour1().r()));
    m_strapColor.setGreenF(qreal(m_muscle->GetStrap()->colour1().g()));
    m_strapColor.setBlueF(qreal(m_muscle->GetStrap()->colour1().b()));
    m_strapColor.setAlphaF(qreal(m_muscle->GetStrap()->colour1().alpha()));
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

    // this will match TwoPointStrap and NPointStrap
    TwoPointStrap *twoPointStrap = dynamic_cast<TwoPointStrap *>(m_muscle->GetStrap());
    if (twoPointStrap)
    {
        std::vector<PointForce *> *pointForceList = twoPointStrap->GetPointForceList();
        std::vector<pgd::Vector> polyline;
        polyline.reserve(pointForceList->size());
        polyline.push_back(pgd::Vector(pointForceList->at(0)->point[0], pointForceList->at(0)->point[1],
                                       pointForceList->at(0)->point[2]));
        for (size_t i = 2; i < pointForceList->size(); i++)
            polyline.push_back(pgd::Vector(pointForceList->at(i)->point[0], pointForceList->at(i)->point[1],
                                           pointForceList->at(i)->point[2]));
        polyline.push_back(pgd::Vector(pointForceList->at(1)->point[0], pointForceList->at(1)->point[1],
                                       pointForceList->at(1)->point[2]));
        FacetedPolyline *facetedPolyline = new FacetedPolyline(&polyline, m_strapRadius, m_strapNumSegments,
                m_strapColor, this);
        facetedPolyline->setLayer(m_layer);
        facetedPolyline->setEffect(m_effect);
        facetedPolyline->InitialiseEntity();
    }
    else
    {
        CylinderWrapStrap *cylinderWrapStrap = dynamic_cast<CylinderWrapStrap *>(m_muscle->GetStrap());
        if (cylinderWrapStrap)
        {
            std::vector<pgd::Vector> polyline;
            const pgd::Vector *pathCoordinates = cylinderWrapStrap->GetPathCoordinates();
            int numPathCoordinates = cylinderWrapStrap->GetNumPathCoordinates();
            polyline.reserve(size_t(numPathCoordinates));
            if (numPathCoordinates)
            {
                for (size_t i = 0; i < size_t(numPathCoordinates); i++)
                {
                    polyline.push_back(pathCoordinates[i]);
                }
                FacetedPolyline *facetedPolyline = new FacetedPolyline(&polyline, m_strapRadius, m_strapNumSegments,
                        m_strapColor, this);
                facetedPolyline->setLayer(m_layer);
                facetedPolyline->setEffect(m_effect);
                facetedPolyline->InitialiseEntity();
            }

            // calculate the quaternion that rotates from cylinder coordinates to world coordinates
            Body *body;
            dVector3 pos;
            dQuaternion qq;
            double radius;
            cylinderWrapStrap->GetCylinder(&body, pos, &radius, qq);
            const double *q = dBodyGetQuaternion(body->GetBodyID());
            pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
            pgd::Quaternion cylinderToWorldQuaternion =  qBody * pgd::Quaternion(qq[0], qq[1], qq[2], qq[3]);
            pgd::Vector cylinderVecWorld = pgd::QVRotate(cylinderToWorldQuaternion, pgd::Vector(0, 0,
                                           m_strapCylinderLength / 2));
            // calculate the cylinder world position
            dVector3 position;
            dBodyGetRelPointPos(body->GetBodyID(), pos[0], pos[1], pos[2], position);
            // and draw it
            polyline.clear();
            polyline.push_back(pgd::Vector(position[0] - cylinderVecWorld.x, position[1] - cylinderVecWorld.y,
                                           position[2] - cylinderVecWorld.z));
            polyline.push_back(pgd::Vector(position[0] + cylinderVecWorld.x, position[1] + cylinderVecWorld.y,
                                           position[2] + cylinderVecWorld.z));
            FacetedPolyline *facetedPolyline = new FacetedPolyline(&polyline, radius, m_strapNumSegments,
                    m_strapCylinderColor, this);
            facetedPolyline->setLayer(m_layer);
            facetedPolyline->setEffect(m_effect);
            facetedPolyline->InitialiseEntity();
        }
    }

    if (m_displayMuscleForces)
    {
        std::vector<PointForce *> *pointForceList = m_muscle->GetPointForceList();
        for (size_t i = 0; i < pointForceList->size(); i++)
        {
            std::vector<pgd::Vector> polyline;
            polyline.reserve(2);
            polyline.clear();
            pgd::Vector f = pgd::Vector(pointForceList->at(i)->vector[0], pointForceList->at(i)->vector[1],
                                        pointForceList->at(i)->vector[2]) * m_muscle->GetTension() * m_strapForceScale;
            polyline.push_back(pgd::Vector(pointForceList->at(i)->point[0], pointForceList->at(i)->point[1],
                                           pointForceList->at(i)->point[2]));
            polyline.push_back(pgd::Vector(pointForceList->at(i)->point[0], pointForceList->at(i)->point[1],
                                           pointForceList->at(i)->point[2]) + f);
            FacetedPolyline *facetedPolyline = new FacetedPolyline(&polyline, m_strapForceRadius,
                    m_strapNumSegments, m_strapForceColor, this);
            facetedPolyline->setLayer(m_layer);
            facetedPolyline->setEffect(m_effect);
            facetedPolyline->InitialiseEntity();
        }
    }

    return 0;
}


