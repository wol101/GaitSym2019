/*
 *  DrawBody.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWBODY_H
#define DRAWBODY_H

#include <QEntity>
#include <QTransform>
#include <QColor>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;
class Body;
class DrawAxes;
class FacetedObject;

class DrawBody : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    DrawBody(Qt3DCore::QNode *parent = nullptr);

    int initialise(Body *body);
    void updateEntityPose();

    Body *body() const;

    QStringList meshSearchPath() const;
    void setMeshSearchPath(const QStringList &meshSearchPath);

    DrawAxes *axes() const;

    FacetedObject *meshEntity1() const;

    FacetedObject *meshEntity2() const;

    FacetedObject *meshEntity3() const;

    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:
    Body *m_body = nullptr;
    QStringList m_meshSearchPath;
    Qt3DCore::QTransform *m_bodyTransform = nullptr;
    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;

    DrawAxes *m_axes = nullptr;
    FacetedObject *m_meshEntity1 = nullptr;
    FacetedObject *m_meshEntity2 = nullptr;
    FacetedObject *m_meshEntity3 = nullptr;

    double m_bodyAxesSize = 0;
    QColor m_bodyColour1;
    QColor m_bodyColour2;
    QColor m_bodyColour3;

};

#endif // DRAWBODY_H
