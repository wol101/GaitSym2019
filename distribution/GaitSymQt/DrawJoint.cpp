/*
 *  DrawJoint.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "DrawJoint.h"
#include "Joint.h"
#include "FacetedObject.h"
#include "Marker.h"
#include "HingeJoint.h"
#include "UniversalJoint.h"
#include "BallJoint.h"
#include "FixedJoint.h"
#include "FacetedConicSegment.h"
#include "FacetedPolyline.h"
#include "PGDMath.h"
#include "Preferences.h"

#include <QString>
#include <QDir>
#include <QDebug>

#include <vector>

DrawJoint::DrawJoint()
{
    m_jointAxisSize = Preferences::valueDouble("JointAxesSize");
    m_jointColor = Preferences::valueQColor("JointColour");
    m_jointSegments = size_t(Preferences::valueInt("JointSegments"));
}

std::string DrawJoint::name()
{
    if (m_joint) return m_joint->name();
    else return std::string();
}

Joint *DrawJoint::joint() const
{
    return m_joint;
}

void DrawJoint::setJoint(Joint *joint)
{
    m_joint = joint;
}

void DrawJoint::initialise(SimulationWidget *simulationWidget)
{
    if (!m_joint) return;

    m_jointColor.setRedF(qreal(m_joint->colour1().r()));
    m_jointColor.setGreenF(qreal(m_joint->colour1().g()));
    m_jointColor.setBlueF(qreal(m_joint->colour1().b()));
    m_jointColor.setAlphaF(qreal(m_joint->colour1().alpha()));
    m_jointAxisSize = m_joint->size1();

    HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_joint);
    if (hingeJoint)
    {
        double halfLen = m_jointAxisSize / 2.0;
        std::vector<pgd::Vector> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector(+halfLen, 0, 0));
        m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        m_facetedObject1->setSimulationWidget(simulationWidget);
        m_facetedObjectList.push_back(m_facetedObject1.get());
        return;
    }

    UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_joint);
    if (universalJoint)
    {
        double halfLen = m_jointAxisSize / 2.0;
        std::vector<pgd::Vector> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector(+halfLen, 0, 0));
        m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        m_facetedObject2 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        m_facetedObject1->setSimulationWidget(simulationWidget);
        m_facetedObject2->setSimulationWidget(simulationWidget);
        m_facetedObjectList.push_back(m_facetedObject1.get());
        m_facetedObjectList.push_back(m_facetedObject2.get());
        return;
    }

    BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_joint);
    if (ballJoint)
    {
        double halfLen = m_jointAxisSize / 2.0;
        std::vector<pgd::Vector> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector(+halfLen, 0, 0));
        m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        polyLine[0] = pgd::Vector(0, -halfLen, 0);
        polyLine[1] = pgd::Vector(0, +halfLen, 0);
        m_facetedObject2 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        polyLine[0] = pgd::Vector(0, 0, -halfLen);
        polyLine[1] = pgd::Vector(0, 0, +halfLen);
        m_facetedObject3 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        m_facetedObject1->setSimulationWidget(simulationWidget);
        m_facetedObject2->setSimulationWidget(simulationWidget);
        m_facetedObject3->setSimulationWidget(simulationWidget);
        m_facetedObjectList.push_back(m_facetedObject1.get());
        m_facetedObjectList.push_back(m_facetedObject2.get());
        m_facetedObjectList.push_back(m_facetedObject3.get());
        return;
    }

    FixedJoint *fixedJoint = dynamic_cast<FixedJoint *>(m_joint);
    if (fixedJoint)
    {
        qDebug() << "Warning in DrawJoint::initialise: FixedJoint is not drawn";
        return;
    }

    qDebug() << "Error in DrawJoint::initialise: Unsupported JOINT type";
}

void DrawJoint::updateEntityPose()
{
    Marker *marker = m_joint->body1Marker();
    pgd::Quaternion q = marker->GetWorldQuaternion();
    pgd::Vector p = marker->GetWorldPosition();
    SetDisplayRotationFromQuaternion(q.data());
    SetDisplayPosition(p.x, p.y, p.z);
}

void DrawJoint::Draw()
{
    if (m_facetedObject1.get()) m_facetedObject1->Draw();
    if (m_facetedObject2.get()) m_facetedObject2->Draw();
    if (m_facetedObject3.get()) m_facetedObject3->Draw();
}



