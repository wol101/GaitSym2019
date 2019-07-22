/*
 *  DrawJoint.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWJOINT_H
#define DRAWJOINT_H

#include <QEntity>
#include <QTransform>
#include <QColor>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;
class Joint;

class DrawJoint : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    DrawJoint(Qt3DCore::QNode *parent = nullptr);

    int initialise(Joint *joint);
    void updateEntityPose();

    Joint *joint() const;

    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:
    Joint *m_joint = nullptr;
    Qt3DCore::QTransform *m_jointTransform = nullptr;
    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;

    double m_jointAxisSize;
    QColor m_jointColor;
    int m_jointSegments;

};

#endif // DRAWJOINT_H
