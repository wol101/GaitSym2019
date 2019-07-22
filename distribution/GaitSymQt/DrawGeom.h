/*
 *  DrawGeom.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 11/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWGEOM_H
#define DRAWGEOM_H

#include <QEntity>
#include <QTransform>
#include <QColor>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;
class Geom;

class DrawGeom : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    DrawGeom(Qt3DCore::QNode *parent = nullptr);

    int initialise(Geom *geom);
    void updateEntityPose();

    Geom *geom() const;

    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:
    Geom *m_geom = nullptr;
    Qt3DCore::QTransform *m_geomTransform = nullptr;
    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;

    QColor m_geomColor1;
    QColor m_geomColor2;
    int m_geomSegments = 0;

};

#endif // DRAWGEOM_H
