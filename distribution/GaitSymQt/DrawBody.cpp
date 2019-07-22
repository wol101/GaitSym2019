/*
 *  DrawBody.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "DrawBody.h"
#include "Body.h"
#include "DrawAxes.h"
#include "FacetedObject.h"
#include "Preferences.h"

#include <QString>
#include <QDir>
#include <QMesh>
#include <QTransform>
#include <QDiffuseSpecularMaterial>

DrawBody::DrawBody(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
    m_meshSearchPath.append(".");
    m_bodyAxesSize = Preferences::valueDouble("BodyAxesSize");
    m_bodyColour1 = Preferences::valueQColor("BodyColour1");
    m_bodyColour2 = Preferences::valueQColor("BodyColour2");
    m_bodyColour3 = Preferences::valueQColor("BodyColour3");
}

Body *DrawBody::body() const
{
    return m_body;
}

QStringList DrawBody::meshSearchPath() const
{
    return m_meshSearchPath;
}

void DrawBody::setMeshSearchPath(const QStringList &meshSearchPath)
{
    m_meshSearchPath = meshSearchPath;
}

DrawAxes *DrawBody::axes() const
{
    return m_axes;
}

FacetedObject *DrawBody::meshEntity1() const
{
    return m_meshEntity1;
}

FacetedObject *DrawBody::meshEntity2() const
{
    return m_meshEntity2;
}

FacetedObject *DrawBody::meshEntity3() const
{
    return m_meshEntity3;
}

Qt3DRender::QLayer *DrawBody::layer() const
{
    return m_layer;
}

void DrawBody::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

SceneEffect *DrawBody::effect() const
{
    return m_effect;
}

void DrawBody::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}

int DrawBody::initialise(Body *body)
{
    if (!body) return __LINE__;

    // create an entity to hold the body transformation
    m_body = body;
    m_bodyColour1.setRedF(qreal(m_body->colour1().r()));
    m_bodyColour1.setGreenF(qreal(m_body->colour1().g()));
    m_bodyColour1.setBlueF(qreal(m_body->colour1().b()));
    m_bodyColour1.setAlphaF(qreal(m_body->colour1().alpha()));
    m_bodyColour2.setRedF(qreal(m_body->colour2().r()));
    m_bodyColour2.setGreenF(qreal(m_body->colour2().g()));
    m_bodyColour2.setBlueF(qreal(m_body->colour2().b()));
    m_bodyColour2.setAlphaF(qreal(m_body->colour2().alpha()));
    m_bodyColour3.setRedF(qreal(m_body->colour3().r()));
    m_bodyColour3.setGreenF(qreal(m_body->colour3().g()));
    m_bodyColour3.setBlueF(qreal(m_body->colour3().b()));
    m_bodyColour3.setAlphaF(qreal(m_body->colour3().alpha()));

    m_bodyTransform = new Qt3DCore::QTransform(this);
    const double *quaternion = m_body->GetQuaternion();
    m_bodyTransform->setRotation(QQuaternion(float(quaternion[0]), float(quaternion[1]),
                                 float(quaternion[2]), float(quaternion[3])));
    const double *position = m_body->GetPosition();
    m_bodyTransform->setTranslation(QVector3D(float(position[0]), float(position[1]),
                                    float(position[2])));
    this->addComponent(m_bodyTransform);

    m_meshSearchPath = QString::fromStdString(
                           body->simulation()->GetGlobal().MeshSearchPath()).split(':');
    m_bodyAxesSize = body->size1();

    // add the axes
    m_axes = new DrawAxes(this);
    m_axes->setLayer(m_layer);
    m_axes->setEffect(m_effect);
    Qt3DCore::QTransform *axesTransform = new Qt3DCore::QTransform(m_axes);
    axesTransform->setScale3D(QVector3D(float(m_bodyAxesSize), float(m_bodyAxesSize),
                                        float(m_bodyAxesSize)));
    m_axes->addComponent(axesTransform);

    m_meshEntity1 = new FacetedObject(this);
    m_meshEntity1->setLayer(m_layer);
    m_meshEntity1->setEffect(m_effect);
    QString filename = QString::fromStdString(m_body->GetGraphicFile1());
    if (filename.size())
    {
        for (int i = 0; i < m_meshSearchPath.size(); i++)
        {
            QDir dir(m_meshSearchPath[i]);
            if (dir.exists(filename))
            {
                m_meshEntity1->SetColour(m_bodyColour1);
                if (filename.endsWith(".ply", Qt::CaseInsensitive)) m_meshEntity1->ParsePLYFile(
                        dir.absoluteFilePath(filename).toStdString());
                if (filename.endsWith(".obj", Qt::CaseInsensitive)) m_meshEntity1->ParseOBJFile(
                        dir.absoluteFilePath(filename).toStdString());
                m_meshEntity1->InitialiseEntity();
                Qt3DCore::QTransform *meshTransform = new Qt3DCore::QTransform(m_meshEntity1);
                const double *offset = m_body->GetConstructionPosition();
                meshTransform->setTranslation(QVector3D(float(-offset[0]), float(-offset[1]), float(-offset[2])));
                m_meshEntity1->addComponent(meshTransform);
                break;
            }
        }
    }
    m_meshEntity2 = new FacetedObject(this);
    m_meshEntity2->setLayer(m_layer);
    m_meshEntity2->setEffect(m_effect);
    filename = QString::fromStdString(m_body->GetGraphicFile2());
    if (filename.size())
    {
        for (int i = 0; i < m_meshSearchPath.size(); i++)
        {
            QDir dir(m_meshSearchPath[i]);
            if (dir.exists(filename))
            {
                m_meshEntity2->SetColour(m_bodyColour2);
                if (filename.endsWith(".ply", Qt::CaseInsensitive)) m_meshEntity2->ParsePLYFile(
                        dir.absoluteFilePath(filename).toStdString());
                if (filename.endsWith(".obj", Qt::CaseInsensitive)) m_meshEntity2->ParseOBJFile(
                        dir.absoluteFilePath(filename).toStdString());
                m_meshEntity2->InitialiseEntity();
                Qt3DCore::QTransform *meshTransform = new Qt3DCore::QTransform(m_meshEntity2);
                const double *offset = m_body->GetConstructionPosition();
                meshTransform->setTranslation(QVector3D(float(-offset[0]), float(-offset[1]), float(-offset[2])));
                m_meshEntity2->addComponent(meshTransform);
                break;
            }
        }
    }
    m_meshEntity3 = new FacetedObject(this);
    m_meshEntity3->setLayer(m_layer);
    m_meshEntity3->setEffect(m_effect);
    filename = QString::fromStdString(m_body->GetGraphicFile3());
    if (filename.size())
    {
        for (int i = 0; i < m_meshSearchPath.size(); i++)
        {
            QDir dir(m_meshSearchPath[i]);
            if (dir.exists(filename))
            {
                m_meshEntity3->SetColour(m_bodyColour3);
                if (filename.endsWith(".ply", Qt::CaseInsensitive)) m_meshEntity3->ParsePLYFile(
                        dir.absoluteFilePath(filename).toStdString());
                if (filename.endsWith(".obj", Qt::CaseInsensitive)) m_meshEntity3->ParseOBJFile(
                        dir.absoluteFilePath(filename).toStdString());
                m_meshEntity3->InitialiseEntity();
                Qt3DCore::QTransform *meshTransform = new Qt3DCore::QTransform(m_meshEntity3);
                const double *offset = m_body->GetConstructionPosition();
                meshTransform->setTranslation(QVector3D(float(-offset[0]), float(-offset[1]), float(-offset[2])));
                m_meshEntity3->addComponent(meshTransform);
                break;
            }
        }
    }

    return 0;
}

void DrawBody::updateEntityPose()
{
    const double *quaternion = m_body->GetQuaternion();
    m_bodyTransform->setRotation(QQuaternion(float(quaternion[0]), float(quaternion[1]),
                                 float(quaternion[2]), float(quaternion[3])));
    const double *position = m_body->GetPosition();
    m_bodyTransform->setTranslation(QVector3D(float(position[0]), float(position[1]),
                                    float(position[2])));
}

