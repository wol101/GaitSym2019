/*
 *  DrawMarker.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 11/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWMARKER_H
#define DRAWMARKER_H

#include <QEntity>
#include <QTransform>
#include <QColor>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;
class Marker;

class DrawMarker : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit DrawMarker(Qt3DCore::QNode *parent = nullptr);

    int initialise(Marker *marker);
    void updateEntityPose();

    Marker *marker() const;

    double radius() const;
    void setRadius(double radius);

    int level() const;
    void setLevel(int level);

    QColor color() const;
    void setColor(const QColor &color);

    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:
    Marker *m_marker;
    Qt3DCore::QTransform *m_markerTransform;
    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;

    double m_radius;
    int m_level;
    QColor m_color;
};

#endif // DRAWMARKER_H
