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
#include "DrawAxes.h"
#include "FacetedObject.h"
#include "Marker.h"
#include "HingeJoint.h"
#include "UniversalJoint.h"
#include "BallJoint.h"
#include "FacetedConicSegment.h"
#include "FacetedPolyline.h"
#include "PGDMath.h"
#include "Preferences.h"

#include <QString>
#include <QDir>
#include <QMesh>
#include <QTransform>
#include <QDiffuseSpecularMaterial>

#include <vector>

DrawJoint::DrawJoint(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
    m_joint = nullptr;
    m_jointTransform = nullptr;
    m_jointAxisSize = Preferences::valueDouble("JointAxesSize");
    m_jointColor = Preferences::valueQColor("JointColour");
    m_jointSegments = Preferences::valueInt("JointSegments");
}

Joint *DrawJoint::joint() const
{
    return m_joint;
}

Qt3DRender::QLayer *DrawJoint::layer() const
{
    return m_layer;
}

void DrawJoint::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

SceneEffect *DrawJoint::effect() const
{
    return m_effect;
}

void DrawJoint::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}

int DrawJoint::initialise(Joint *joint)
{
    if (!joint) return __LINE__;

    m_joint = joint;
    m_jointColor.setRedF(qreal(m_joint->colour1().r()));
    m_jointColor.setGreenF(qreal(m_joint->colour1().g()));
    m_jointColor.setBlueF(qreal(m_joint->colour1().b()));
    m_jointColor.setAlphaF(qreal(m_joint->colour1().alpha()));
    m_jointAxisSize = m_joint->size1();

    m_jointTransform = new Qt3DCore::QTransform(this);
    this->addComponent(m_jointTransform);

    HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_joint);
    if (hingeJoint)
    {
        double halfLen = m_jointAxisSize / 2.0;
        std::vector<pgd::Vector> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector(+halfLen, 0, 0));
        FacetedPolyline *meshEntity1 = new FacetedPolyline(&polyLine, halfLen / 10, m_jointSegments,
                m_jointColor, this);
        meshEntity1->setLayer(m_layer);
        meshEntity1->setEffect(m_effect);
        meshEntity1->InitialiseEntity();

        Marker *marker = hingeJoint->body1Marker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_jointTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_jointTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
//        double halfLen = m_jointAxisSize / 2.0;
//        FacetedConicSegment *meshEntity = new FacetedConicSegment(halfLen * 2, halfLen / 10, halfLen / 10, m_jointSegments, 0, 0, -halfLen, this);
//        meshEntity->SetColour(m_jointColor);
//        meshEntity->InitialiseEntity();

//        dVector3 anchor;
//        dVector3 axis;
//        hingeJoint->GetHingeAnchor(anchor);
//        hingeJoint->GetHingeAxis(axis);
//        pgd::Quaternion q = pgd::FindRotation(pgd::Vector(0,0,1), pgd::Vector(axis[0], axis[1], axis[2]));
//        m_jointTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
//        m_jointTransform->setTranslation(QVector3D(float(anchor[0]), float(anchor[1]), float(anchor[2])));
        return 0;
    }

    UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_joint);
    if (universalJoint)
    {
        double halfLen = m_jointAxisSize / 2.0;
        std::vector<pgd::Vector> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector(+halfLen, 0, 0));
        FacetedPolyline *meshEntity1 = new FacetedPolyline(&polyLine, halfLen / 10, m_jointSegments,
                m_jointColor, this);
        meshEntity1->setLayer(m_layer);
        meshEntity1->setEffect(m_effect);
        meshEntity1->InitialiseEntity();
        polyLine[0] = pgd::Vector(0, -halfLen, 0);
        polyLine[1] = pgd::Vector(0, +halfLen, 0);
        FacetedPolyline *meshEntity2 = new FacetedPolyline(&polyLine, halfLen / 10, m_jointSegments,
                m_jointColor, this);
        meshEntity2->setLayer(m_layer);
        meshEntity2->setEffect(m_effect);
        meshEntity2->InitialiseEntity();

        Marker *marker = universalJoint->body1Marker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_jointTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_jointTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return 0;
    }

    BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_joint);
    if (ballJoint)
    {
        double halfLen = m_jointAxisSize / 2.0;
        std::vector<pgd::Vector> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector(+halfLen, 0, 0));
        FacetedPolyline *meshEntity1 = new FacetedPolyline(&polyLine, halfLen / 10, m_jointSegments,
                m_jointColor, this);
        meshEntity1->setLayer(m_layer);
        meshEntity1->setEffect(m_effect);
        meshEntity1->InitialiseEntity();
        polyLine[0] = pgd::Vector(0, -halfLen, 0);
        polyLine[1] = pgd::Vector(0, +halfLen, 0);
        FacetedPolyline *meshEntity2 = new FacetedPolyline(&polyLine, halfLen / 10, m_jointSegments,
                m_jointColor, this);
        meshEntity2->setLayer(m_layer);
        meshEntity2->setEffect(m_effect);
        meshEntity2->InitialiseEntity();
        polyLine[0] = pgd::Vector(0, 0, -halfLen);
        polyLine[1] = pgd::Vector(0, 0, +halfLen);
        FacetedPolyline *meshEntity3 = new FacetedPolyline(&polyLine, halfLen / 10, m_jointSegments,
                m_jointColor, this);
        meshEntity3->setLayer(m_layer);
        meshEntity3->setEffect(m_effect);
        meshEntity3->InitialiseEntity();

        Marker *marker = ballJoint->body1Marker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_jointTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_jointTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return 0;
    }

    return 0;
}

void DrawJoint::updateEntityPose()
{
    HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_joint);
    if (hingeJoint)
    {
        Marker *marker = hingeJoint->body1Marker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_jointTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_jointTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return;
    }

    UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_joint);
    if (universalJoint)
    {
        Marker *marker = universalJoint->body1Marker();
        pgd::Quaternion q = marker->GetWorldQuaternion();
        pgd::Vector v = marker->GetWorldPosition();
        m_jointTransform->setRotation(QQuaternion(float(q.n), float(q.v.x), float(q.v.y), float(q.v.z)));
        m_jointTransform->setTranslation(QVector3D(float(v.x), float(v.y), float(v.z)));
        return;
    }
}

