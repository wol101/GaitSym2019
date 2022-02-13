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

#include "Drawable.h"

#include "Colour.h"

#include <QColor>

#include <vector>
#include <memory>

class Muscle;
class FacetedObject;
class SimulationWidget;

class DrawMuscle : public Drawable
{
public:
    DrawMuscle();
    virtual ~DrawMuscle();

    virtual void initialise(SimulationWidget *simulationWidget);
    virtual void Draw();
    virtual std::string name();

    Muscle *muscle() const;
    void setMuscle(Muscle *muscle);

    double strapRadius() const;
    void setStrapRadius(double strapRadius);

    size_t strapNumSegments() const;
    void setStrapNumSegments(size_t strapNumSegments);

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


    Colour::ColourMap strapColourMap() const;
    void setStrapColourMap(const Colour::ColourMap &strapColourMap);

private:
    Muscle *m_muscle = nullptr;

    std::unique_ptr<FacetedObject> m_facetedObject1;
    std::unique_ptr<FacetedObject> m_facetedObject2;
    std::unique_ptr<FacetedObject> m_facetedObject3;
    std::vector<std::unique_ptr<FacetedObject>> m_facetedObjectForceList;

    double m_strapRadius = 0;
    size_t m_strapNumSegments = 0;
    double m_strapCylinderLength = 0;
    size_t m_strapCylinderSegments = 0;
    size_t m_strapCylinderWrapSegments = 0;
    bool m_displayMuscleForces = 0;
    double m_strapForceScale = 0;
    double m_strapForceRadius = 0;
    QColor m_strapForceColor;
    QColor m_strapColor;
    QColor m_strapCylinderColor;
    Colour::ColourMap m_strapColourMap = Colour::ColourMap::JetColourMap;
};

#endif // DRAWMUSCLE_H
