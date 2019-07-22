/*
 *  DrawAxes.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "DrawAxes.h"

#include "FacetedBox.h"
#include "FacetedConicSegment.h"
#include "FacetedConicSegment.h"

//#include <QCuboidMesh>
//#include <QDiffuseSpecularMaterial>
//#include <QTransform>
//#include <QCylinderMesh>
//#include <QConeMesh>


DrawAxes::DrawAxes(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
}

void DrawAxes::initialise()
{
    // the central cube
    float widthFraction = 0.1f;
    FacetedBox *cuboid = new FacetedBox(widthFraction, widthFraction, widthFraction, QColor(255, 220,
                                        220, 220), this);
    cuboid->InitialiseEntity();
    Qt3DCore::QTransform *cuboidTransform = new Qt3DCore::QTransform(this);
    cuboid->addComponent(cuboidTransform);

    int tesselationCylinder = 64;
    int tesselationCone = 128;
    float height = 1;
    float lengthCylinder = height * (1 - 2 * widthFraction);
    float heightCone = widthFraction;
    float widthCylinder = height * 0.25f * widthFraction;
    float widthCone = height * widthFraction * 0.5f;

    QColor colorCylinder1(255, 0, 0, 255); // red
    FacetedConicSegment *cylinder1 = new FacetedConicSegment(lengthCylinder, widthCylinder,
            widthCylinder, tesselationCylinder, 0, 0, -lengthCylinder / 2, colorCylinder1, this);
    cylinder1->InitialiseEntity();
    Qt3DCore::QTransform *cylinder1Transform = new Qt3DCore::QTransform(this);
    cylinder1Transform->setRotationY(90);
    cylinder1Transform->setTranslation(QVector3D(widthFraction * 0.5f + lengthCylinder * 0.5f, 0, 0));
    cylinder1->addComponent(cylinder1Transform);

    FacetedConicSegment *cone1 = new FacetedConicSegment(heightCone, widthCone, 0, tesselationCone, 0,
            0, -heightCone / 2, colorCylinder1, this);
    Qt3DCore::QTransform *cone1Transform = new Qt3DCore::QTransform(this);
    cone1->InitialiseEntity();
    cone1Transform->setRotationY(90);
    cone1Transform->setTranslation(QVector3D(1 - heightCone, 0, 0));
    cone1->addComponent(cone1Transform);

    QColor colorCylinder2(0, 255, 0, 255); // green
    FacetedConicSegment *cylinder2 = new FacetedConicSegment(lengthCylinder, widthCylinder,
            widthCylinder, tesselationCylinder, 0, 0, -lengthCylinder / 2, colorCylinder2, this);
    cylinder2->InitialiseEntity();
    Qt3DCore::QTransform *cylinder2Transform = new Qt3DCore::QTransform(this);
    cylinder2Transform->setRotationX(-90);
    cylinder2Transform->setTranslation(QVector3D(0, widthFraction * 0.5f + lengthCylinder * 0.5f, 0));
    cylinder2->addComponent(cylinder2Transform);

    FacetedConicSegment *cone2 = new FacetedConicSegment(heightCone, widthCone, 0, tesselationCone, 0,
            0, -heightCone / 2, colorCylinder2, this);
    Qt3DCore::QTransform *cone2Transform = new Qt3DCore::QTransform(this);
    cone2->InitialiseEntity();
    cone2Transform->setRotationX(-90);
    cone2Transform->setTranslation(QVector3D(0, 1 - heightCone, 0));
    cone2->addComponent(cone2Transform);

    QColor colorCylinder3(0, 0, 255, 255); // blue
    FacetedConicSegment *cylinder3 = new FacetedConicSegment(lengthCylinder, widthCylinder,
            widthCylinder, tesselationCylinder, 0, 0, -lengthCylinder / 2, colorCylinder3, this);
    cylinder3->InitialiseEntity();
    Qt3DCore::QTransform *cylinder3Transform = new Qt3DCore::QTransform(this);
    // cylinder3Transform->setRotationX(90);
    cylinder3Transform->setTranslation(QVector3D(0, 0, widthFraction * 0.5f + lengthCylinder * 0.5f));
    cylinder3->addComponent(cylinder3Transform);

    FacetedConicSegment *cone3 = new FacetedConicSegment(heightCone, widthCone, 0, tesselationCone, 0,
            0, -heightCone / 2, colorCylinder3, this);
    cone3->InitialiseEntity();
    Qt3DCore::QTransform *cone3Transform = new Qt3DCore::QTransform(this);
    // cone3Transform->setRotationX(90);
    cone3Transform->setTranslation(QVector3D(0, 0, 1 - heightCone));
    cone3->addComponent(cone3Transform);

//    // the central cube
//    float widthFraction = 0.1f;
//    Qt3DExtras::QCuboidMesh *cuboidMesh = new Qt3DExtras::QCuboidMesh(this);
//    cuboidMesh->setXExtent(widthFraction * 1);
//    cuboidMesh->setYExtent(widthFraction * 1);
//    cuboidMesh->setZExtent(widthFraction * 1);
//    Qt3DExtras::QDiffuseSpecularMaterial *cuboidMaterial = new Qt3DExtras::QDiffuseSpecularMaterial(this);
//    cuboidMaterial->setDiffuse(QColor(255, 220, 220, 220));
//    Qt3DCore::QTransform *cuboidTransform = new Qt3DCore::QTransform(this);
//    Qt3DCore::QEntity* cuboid = new Qt3DCore::QEntity(this);
//    cuboid->addComponent(cuboidMesh);
//    cuboid->addComponent(cuboidMaterial);
//    cuboid->addComponent(cuboidTransform);

//    int tesselationCylinder = 64;
//    int tesselationCone = 128;
//    float height = 1;
//    float lengthCylinder = height * (1 - 2 * widthFraction);
//    float heightCone = widthFraction;
//    float widthCylinder = height * 0.25f * widthFraction;
//    float widthCone = height * widthFraction * 0.5f;

//    QColor colorCylinder1(255, 0, 0, 255); // red
//    Qt3DExtras::QCylinderMesh *cylinder1Mesh = new Qt3DExtras::QCylinderMesh(this);
//    cylinder1Mesh->setLength(lengthCylinder);
//    cylinder1Mesh->setRadius(widthCylinder);
//    cylinder1Mesh->setSlices(tesselationCylinder);
//    Qt3DExtras::QDiffuseSpecularMaterial *cylinder1Material = new Qt3DExtras::QDiffuseSpecularMaterial(this);
//    cylinder1Material->setDiffuse(colorCylinder1);
//    Qt3DCore::QTransform *cylinder1Transform = new Qt3DCore::QTransform(this);
//    cylinder1Transform->setRotationZ(-90);
//    cylinder1Transform->setTranslation(QVector3D(widthFraction * 0.5f + lengthCylinder * 0.5f, 0, 0));
//    Qt3DCore::QEntity* cylinder1 = new Qt3DCore::QEntity(this);
//    cylinder1->addComponent(cylinder1Mesh);
//    cylinder1->addComponent(cylinder1Material);
//    cylinder1->addComponent(cylinder1Transform);

//    Qt3DExtras::QConeMesh *cone1Mesh = new Qt3DExtras::QConeMesh(this);
//    cone1Mesh->setLength(heightCone);
//    cone1Mesh->setBottomRadius(widthCone);
//    cone1Mesh->setSlices(tesselationCone);
//    Qt3DExtras::QDiffuseSpecularMaterial *cone1Material = new Qt3DExtras::QDiffuseSpecularMaterial(this);
//    cone1Material->setDiffuse(colorCylinder1);
//    Qt3DCore::QTransform *cone1Transform = new Qt3DCore::QTransform(this);
//    cone1Transform->setRotationZ(-90);
//    cone1Transform->setTranslation(QVector3D(1 - heightCone, 0, 0));
//    Qt3DCore::QEntity* cone1 = new Qt3DCore::QEntity(this);
//    cone1->addComponent(cone1Mesh);
//    cone1->addComponent(cone1Material);
//    cone1->addComponent(cone1Transform);

//    QColor colorCylinder2(0, 255, 0, 255); // green
//    Qt3DExtras::QCylinderMesh *cylinder2Mesh = new Qt3DExtras::QCylinderMesh(this);
//    cylinder2Mesh->setLength(lengthCylinder);
//    cylinder2Mesh->setRadius(widthCylinder);
//    cylinder2Mesh->setSlices(tesselationCylinder);
//    Qt3DExtras::QDiffuseSpecularMaterial *cylinder2Material = new Qt3DExtras::QDiffuseSpecularMaterial(this);
//    cylinder2Material->setDiffuse(colorCylinder2);
//    Qt3DCore::QTransform *cylinder2Transform = new Qt3DCore::QTransform(this);
//    // cylinder2Transform->setRotationY(0);
//    cylinder2Transform->setTranslation(QVector3D(0, widthFraction * 0.5f + lengthCylinder * 0.5f, 0));
//    Qt3DCore::QEntity* cylinder2 = new Qt3DCore::QEntity(this);
//    cylinder2->addComponent(cylinder2Mesh);
//    cylinder2->addComponent(cylinder2Material);
//    cylinder2->addComponent(cylinder2Transform);

//    Qt3DExtras::QConeMesh *cone2Mesh = new Qt3DExtras::QConeMesh(this);
//    cone2Mesh->setLength(heightCone);
//    cone2Mesh->setBottomRadius(widthCone);
//    cone2Mesh->setSlices(tesselationCone);
//    Qt3DExtras::QDiffuseSpecularMaterial *cone2Material = new Qt3DExtras::QDiffuseSpecularMaterial(this);
//    cone2Material->setDiffuse(colorCylinder2);
//    Qt3DCore::QTransform *cone2Transform = new Qt3DCore::QTransform(this);
//    // cone2Transform->setRotationY(0);
//    cone2Transform->setTranslation(QVector3D(0, 1 - heightCone, 0));
//    Qt3DCore::QEntity* cone2 = new Qt3DCore::QEntity(this);
//    cone2->addComponent(cone2Mesh);
//    cone2->addComponent(cone2Material);
//    cone2->addComponent(cone2Transform);

//    QColor colorCylinder3(0, 0, 255, 255); // blue
//    Qt3DExtras::QCylinderMesh *cylinder3Mesh = new Qt3DExtras::QCylinderMesh(this);
//    cylinder3Mesh->setLength(lengthCylinder);
//    cylinder3Mesh->setRadius(widthCylinder);
//    cylinder3Mesh->setSlices(tesselationCylinder);
//    Qt3DExtras::QDiffuseSpecularMaterial *cylinder3Material = new Qt3DExtras::QDiffuseSpecularMaterial(this);
//    cylinder3Material->setDiffuse(colorCylinder3);
//    Qt3DCore::QTransform *cylinder3Transform = new Qt3DCore::QTransform(this);
//    cylinder3Transform->setRotationX(90);
//    cylinder3Transform->setTranslation(QVector3D(0, 0, widthFraction * 0.5f + lengthCylinder * 0.5f));
//    Qt3DCore::QEntity* cylinder3 = new Qt3DCore::QEntity(this);
//    cylinder3->addComponent(cylinder3Mesh);
//    cylinder3->addComponent(cylinder3Material);
//    cylinder3->addComponent(cylinder3Transform);

//    Qt3DExtras::QConeMesh *cone3Mesh = new Qt3DExtras::QConeMesh(this);
//    cone3Mesh->setLength(heightCone);
//    cone3Mesh->setBottomRadius(widthCone);
//    cone3Mesh->setSlices(tesselationCone);
//    Qt3DExtras::QDiffuseSpecularMaterial *cone3Material = new Qt3DExtras::QDiffuseSpecularMaterial(this);
//    cone3Material->setDiffuse(colorCylinder3);
//    Qt3DCore::QTransform *cone3Transform = new Qt3DCore::QTransform(this);
//    cone3Transform->setRotationX(90);
//    cone3Transform->setTranslation(QVector3D(0, 0, 1 - heightCone));
//    Qt3DCore::QEntity* cone3 = new Qt3DCore::QEntity(this);
//    cone3->addComponent(cone3Mesh);
//    cone3->addComponent(cone3Material);
//    cone3->addComponent(cone3Transform);

}

Qt3DRender::QLayer *DrawAxes::layer() const
{
    return m_layer;
}

void DrawAxes::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

SceneEffect *DrawAxes::effect() const
{
    return m_effect;
}

void DrawAxes::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}
