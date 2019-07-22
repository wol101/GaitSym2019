/*
 *  DrawFluidSac.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWFLUIDSAC_H
#define DRAWFLUIDSAC_H

#include <QEntity>
#include <QTransform>
#include <QColor>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;
class FluidSac;

class DrawFluidSac : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    DrawFluidSac(Qt3DCore::QNode *parent = nullptr);

    int initialise(FluidSac *fluidSac);

    FluidSac *fluidSac() const;

    QColor fluidSacColour() const;
    void setFluidSacColour(const QColor &fluidSacColour);

    bool displayFluidSacForces() const;
    void setDisplayFluidSacForces(bool displayFluidSacForces);

    double fluidSacForceScale() const;
    void setFluidSacForceScale(double fluidSacForceScale);

    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:
    FluidSac *m_fluidSac = nullptr;
    Qt3DCore::QTransform *m_fluidSacTransform = nullptr;
    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;


    QColor m_fluidSacColour;
    QColor m_fluidSacForceColour;
    bool m_displayFluidSacForces;
    double m_fluidSacForceScale;
    double m_fluidSacForceRadius;
    int m_fluidSacForceSegments;
};

#endif // DRAWFLUIDSAC_H
