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
#include "FacetedRect.h"
#include "PGDMath.h"
#include "Preferences.h"
#include "Colour.h"

#include <QString>
#include <QDir>
#include <QDebug>
#include <QOpenGLTexture>
#include <QOpenGLPixelTransferOptions>

#include <vector>
#include <memory>

DrawJoint::DrawJoint()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    m_objectCountAtCreation = m_objectCount++;
    std::cerr << m_objectCountAtCreation << " " << className() << " constructed\n";;
#endif
    m_jointAxisSize = Preferences::valueDouble("JointAxesSize");
    m_jointColor = Preferences::valueQColor("JointColour");
    m_jointSegments = size_t(Preferences::valueInt("JointSegments"));
}

DrawJoint::~DrawJoint()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    std::cerr << m_objectCountAtCreation << " " << className() << " destructed\n";;
#endif
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
        std::vector<pgd::Vector3> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector3(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector3(+halfLen, 0, 0));
        m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        m_facetedObject1->setSimulationWidget(simulationWidget);
        m_facetedObjectList.push_back(m_facetedObject1.get());
        return;
    }

    UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_joint);
    if (universalJoint)
    {
        double halfLen = m_jointAxisSize / 2.0;
        std::vector<pgd::Vector3> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector3(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector3(+halfLen, 0, 0));
        m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        polyLine[0] = pgd::Vector3(0, -halfLen, 0);
        polyLine[1] = pgd::Vector3(0, +halfLen, 0);
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
        std::vector<pgd::Vector3> polyLine;
        polyLine.reserve(2);
        polyLine.push_back(pgd::Vector3(-halfLen, 0, 0));
        polyLine.push_back(pgd::Vector3(+halfLen, 0, 0));
        m_facetedObject1 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        polyLine[0] = pgd::Vector3(0, -halfLen, 0);
        polyLine[1] = pgd::Vector3(0, +halfLen, 0);
        m_facetedObject2 = std::make_unique<FacetedPolyline>(&polyLine, halfLen / 10, m_jointSegments, m_jointColor, 1);
        polyLine[0] = pgd::Vector3(0, 0, -halfLen);
        polyLine[1] = pgd::Vector3(0, 0, +halfLen);
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
    if (fixedJoint && fixedJoint->GetStressCalculationType() != FixedJoint::none)
    {
        qDebug() << "Debug DrawJoint::initialise:" << m_joint->name().c_str();
        m_facetedObject1 = std::make_unique<FacetedRect>(fixedJoint->width(), fixedJoint->height(), m_jointColor, 1);
        m_facetedObject1->setSimulationWidget(simulationWidget);
        m_facetedObject1->Move((fixedJoint->width() / 2) - fixedJoint->xOrigin(), (fixedJoint->height() / 2) - fixedJoint->yOrigin(), 0);
        fixedJoint->CalculatePixmap();
        std::unique_ptr<QOpenGLTexture> texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        texture->setAutoMipMapGenerationEnabled(false);
        texture->setFormat(QOpenGLTexture::RGBA8_UNorm); // this maps to QImage::Format_RGBA8888
        texture->setSize(int(fixedJoint->nx()), int(fixedJoint->ny()), 1);
        texture->setMipLevels(1);
        texture->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
        QOpenGLPixelTransferOptions uploadOptions;
        uploadOptions.setAlignment(1);
        texture->setData(0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, fixedJoint->pixMap().data(), &uploadOptions);
        texture->setMinificationFilter(QOpenGLTexture::Nearest);
        texture->setMagnificationFilter(QOpenGLTexture::Nearest);
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        m_facetedObject1->setTexture(std::move(texture));
        m_facetedObject1->setDecal(1);
        m_facetedObjectList.push_back(m_facetedObject1.get());
        return;
    }

    qDebug() << "Error in DrawJoint::initialise: Unsupported JOINT type \"" << m_joint->name().c_str() << "\"";
}

void DrawJoint::updateEntityPose()
{
    Marker *marker = m_joint->body1Marker();
    pgd::Quaternion q = marker->GetWorldQuaternion();
    pgd::Vector3 p = marker->GetWorldPosition();
    SetDisplayRotationFromQuaternion(q.data());
    SetDisplayPosition(p.x, p.y, p.z);
    FixedJoint *fixedJoint = dynamic_cast<FixedJoint *>(m_joint);
    if (fixedJoint && fixedJoint->CalculatePixmapNeeded() && m_facetedObject1->texture())
    {
        fixedJoint->CalculatePixmap();
        QOpenGLPixelTransferOptions uploadOptions;
        uploadOptions.setAlignment(1);
        m_facetedObject1->texture()->setData(0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, fixedJoint->pixMap().data(), &uploadOptions);
    }
}

void DrawJoint::Draw()
{
    if (m_facetedObject1.get()) m_facetedObject1->Draw();
    if (m_facetedObject2.get()) m_facetedObject2->Draw();
    if (m_facetedObject3.get()) m_facetedObject3->Draw();
    m_joint->setRedraw(false);
}



