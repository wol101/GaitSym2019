/*
 *  DrawMuscle.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWMUSCLE_H
#define DRAWMUSCLE_H

#include <QEntity>
#include <QTransform>
#include <QColor>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;
class Muscle;

class DrawMuscle : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    DrawMuscle(Qt3DCore::QNode *parent = nullptr);

    int initialise(Muscle *muscle);

    Muscle *muscle() const;

    double strapRadius() const;
    void setStrapRadius(double strapRadius);

    int strapNumSegments() const;
    void setStrapNumSegments(int strapNumSegments);

    double strapCylinderLength() const;
    void setStrapCylinderLength(double strapCylinderLength);

    bool displayMuscleForces() const;
    void setDisplayMuscleForces(bool displayMuscleForces);

    double strapForceScale() const;
    void setStrapForceScale(double strapForceScale);

    double strapForceRadius() const;
    void setStrapForceRadius(double strapForceRadius);

    QColor strapForceColor() const;
    void setStrapForceColor(const QColor &strapForceColor);

    QColor strapColor() const;
    void setStrapColor(const QColor &strapColor);

    QColor strapCylinderColor() const;
    void setStrapCylinderColor(const QColor &strapCylinderColor);

    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:
    Muscle *m_muscle = nullptr;
    Qt3DCore::QTransform *m_muscleTransform = nullptr;
    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;

    double m_strapRadius;
    int m_strapNumSegments;
    double m_strapCylinderLength;
    bool m_displayMuscleForces;
    double m_strapForceScale;
    double m_strapForceRadius;
    QColor m_strapForceColor;
    QColor m_strapColor;
    QColor m_strapCylinderColor;
};

#endif // DRAWMUSCLE_H
