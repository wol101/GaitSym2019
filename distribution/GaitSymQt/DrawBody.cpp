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
#include "FacetedAxes.h"
#include "FacetedObject.h"
#include "Preferences.h"

#include <QString>
#include <QDir>

DrawBody::DrawBody()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    m_objectCountAtCreation = m_objectCount++;
    std::cerr << m_objectCountAtCreation << " " << className() << " constructed\n";;
#endif
    m_meshSearchPath.append(".");
    m_bodyAxesSize = Preferences::valueDouble("BodyAxesSize");
    m_bodyColour1 = Preferences::valueQColor("BodyColour1");
    m_bodyColour2 = Preferences::valueQColor("BodyColour2");
    m_bodyColour3 = Preferences::valueQColor("BodyColour3");
}

DrawBody::~DrawBody()
{
#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
    std::cerr << m_objectCountAtCreation << " " << className() << " destructed\n";;
#endif
}

std::string DrawBody::name()
{
    if (m_body) return m_body->name();
    else return std::string();
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

FacetedObject *DrawBody::meshEntity1() const
{
    return m_meshEntity1.get();
}

FacetedObject *DrawBody::meshEntity2() const
{
    return m_meshEntity2.get();
}

FacetedObject *DrawBody::meshEntity3() const
{
    return m_meshEntity3.get();
}

FacetedObject *DrawBody::axes() const
{
    return m_axes.get();
}

void DrawBody::setBody(Body *body)
{
    m_body = body;
}

void DrawBody::initialise(SimulationWidget *simulationWidget)
{
    if (!m_body) return;
    // assign the deafult colours
    // note these can be overwritten by the colours in the mesh files
    // and they can act to blend with the mesh file colours too
    // also the mesh cache means that the same mesh will only be coloured once
    // which can produce unexpected effects if reusing meshes
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

    m_meshSearchPath.clear();
    for (size_t i = 0; i < m_body->simulation()->GetGlobal()->MeshSearchPath()->size(); i++)
        m_meshSearchPath.append(QString::fromStdString(m_body->simulation()->GetGlobal()->MeshSearchPath()->at(i)));
    m_bodyAxesSize = m_body->size1();

    // add the axes
    m_axes = std::make_unique<FacetedAxes>();
    m_axes->setSimulationWidget(simulationWidget);

    m_meshEntity1 = std::make_unique<FacetedObject>();
    m_meshEntity1->setSimulationWidget(simulationWidget);
    QString filename = QString::fromStdString(m_body->GetGraphicFile1());
    QString absoluteFilename;
    if (filename.size())
    {
        if (QDir::isAbsolutePath(filename))
        {
            absoluteFilename = filename;
        }
        else
        {
            for (int i = 0; i < m_meshSearchPath.size(); i++)
            {
                QDir dir(m_meshSearchPath[i]);
                if (dir.exists(filename))
                {
                    absoluteFilename = dir.absoluteFilePath(filename);
                    break;
                }
            }
        }
        m_meshEntity1->setBlendColour(m_bodyColour1, 1);
        if (absoluteFilename.endsWith(".ply", Qt::CaseInsensitive)) m_meshEntity1->ParsePLYFile(absoluteFilename.toStdString());
        if (absoluteFilename.endsWith(".obj", Qt::CaseInsensitive)) m_meshEntity1->ParseOBJFile(absoluteFilename.toStdString());
        const double *offset = m_body->GetConstructionPosition();
        m_meshEntity1->Move(-offset[0], -offset[1], -offset[2]);
    }

    m_meshEntity2 = std::make_unique<FacetedObject>();
    m_meshEntity2->setSimulationWidget(simulationWidget);
    filename = QString::fromStdString(m_body->GetGraphicFile2());
    if (filename.size())
    {
        if (QDir::isAbsolutePath(filename))
        {
            absoluteFilename = filename;
        }
        else
        {
            for (int i = 0; i < m_meshSearchPath.size(); i++)
            {
                QDir dir(m_meshSearchPath[i]);
                if (dir.exists(filename))
                {
                    absoluteFilename = dir.absoluteFilePath(filename);
                    break;
                }
            }
        }
        m_meshEntity2->setBlendColour(m_bodyColour2, 1);
        if (absoluteFilename.endsWith(".ply", Qt::CaseInsensitive)) m_meshEntity2->ParsePLYFile(absoluteFilename.toStdString());
        if (absoluteFilename.endsWith(".obj", Qt::CaseInsensitive)) m_meshEntity2->ParseOBJFile(absoluteFilename.toStdString());
        const double *offset = m_body->GetConstructionPosition();
        m_meshEntity2->Move(-offset[0], -offset[1], -offset[2]);
    }

    m_meshEntity3 = std::make_unique<FacetedObject>();
    m_meshEntity3->setSimulationWidget(simulationWidget);
    filename = QString::fromStdString(m_body->GetGraphicFile3());
    if (filename.size())
    {
        if (QDir::isAbsolutePath(filename))
        {
            absoluteFilename = filename;
        }
        else
        {
            for (int i = 0; i < m_meshSearchPath.size(); i++)
            {
                QDir dir(m_meshSearchPath[i]);
                if (dir.exists(filename))
                {
                    absoluteFilename = dir.absoluteFilePath(filename);
                    break;
                }
            }
        }
        m_meshEntity3->setBlendColour(m_bodyColour3, 1);
        if (absoluteFilename.endsWith(".ply", Qt::CaseInsensitive)) m_meshEntity3->ParsePLYFile(absoluteFilename.toStdString());
        if (absoluteFilename.endsWith(".obj", Qt::CaseInsensitive)) m_meshEntity3->ParseOBJFile(absoluteFilename.toStdString());
        const double *offset = m_body->GetConstructionPosition();
        m_meshEntity3->Move(-offset[0], -offset[1], -offset[2]);
    }

    m_facetedObjectList.push_back(m_axes.get());
    m_facetedObjectList.push_back(m_meshEntity1.get());
    m_facetedObjectList.push_back(m_meshEntity2.get());
    m_facetedObjectList.push_back(m_meshEntity3.get());
}

void DrawBody::updateEntityPose()
{
    const double *quaternion = m_body->GetQuaternion();
    const double *position = m_body->GetPosition();
    SetDisplayRotationFromQuaternion(quaternion);
    SetDisplayPosition(position[0], position[1], position[2]);
    m_axes->SetDisplayScale(m_body->size1(), m_body->size1(), m_body->size1());
}

void DrawBody::Draw()
{
    m_axes->Draw();
    if (m_meshEntity1->GetNumTriangles())
    {
        m_bodyColour1.setRedF(qreal(m_body->colour1().r()));
        m_bodyColour1.setGreenF(qreal(m_body->colour1().g()));
        m_bodyColour1.setBlueF(qreal(m_body->colour1().b()));
        m_bodyColour1.setAlphaF(qreal(m_body->colour1().alpha()));
        m_meshEntity1->setBlendColour(m_bodyColour1, m_body->size2());
        m_meshEntity1->Draw();
    }
    if (m_meshEntity2->GetNumTriangles())
    {
        m_bodyColour2.setRedF(qreal(m_body->colour2().r()));
        m_bodyColour2.setGreenF(qreal(m_body->colour2().g()));
        m_bodyColour2.setBlueF(qreal(m_body->colour2().b()));
        m_bodyColour2.setAlphaF(qreal(m_body->colour2().alpha()));
        m_meshEntity2->setBlendColour(m_bodyColour2, m_body->size2());
        m_meshEntity2->Draw();
    }
    if (m_meshEntity3->GetNumTriangles())
    {
        m_bodyColour3.setRedF(qreal(m_body->colour3().r()));
        m_bodyColour3.setGreenF(qreal(m_body->colour3().g()));
        m_bodyColour3.setBlueF(qreal(m_body->colour3().b()));
        m_bodyColour3.setAlphaF(qreal(m_body->colour3().alpha()));
        m_meshEntity3->setBlendColour(m_bodyColour3, m_body->size2());
        m_meshEntity3->Draw();
    }
    m_body->setRedraw(false);
}


