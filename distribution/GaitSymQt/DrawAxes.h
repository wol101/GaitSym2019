/*
 *  DrawAxes.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWAXES_H
#define DRAWAXES_H

#include <QEntity>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;

class DrawAxes : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    DrawAxes(Qt3DCore::QNode *parent = nullptr);
    void initialise();

    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:
    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;

};

#endif // DRAWAXES_H
