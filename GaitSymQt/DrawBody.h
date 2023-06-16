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

#include "Drawable.h"

#include <QColor>
#include <QStringList>

#include <memory>

class Body;
class FacetedObject;
class SimulationWidget;

class DrawBody : public Drawable
{
public:
    DrawBody();
    virtual ~DrawBody();

    virtual void initialise(SimulationWidget *simulationWidget);
    virtual void Draw();
    virtual std::string name();

    void updateEntityPose();

    Body *body() const;
    void setBody(Body *body);

    QStringList meshSearchPath() const;
    void setMeshSearchPath(const QStringList &meshSearchPath);

    FacetedObject *meshEntity1() const;

    FacetedObject *meshEntity2() const;

    FacetedObject *meshEntity3() const;

    FacetedObject *axes() const;

private:
    Body *m_body = nullptr;
    QStringList m_meshSearchPath;

    std::unique_ptr<FacetedObject> m_axes;
    std::unique_ptr<FacetedObject> m_meshEntity1;
    std::unique_ptr<FacetedObject> m_meshEntity2;
    std::unique_ptr<FacetedObject> m_meshEntity3;

    double m_bodyAxesSize = 0;
    QColor m_bodyColour1;
    QColor m_bodyColour2;
    QColor m_bodyColour3;

};

#endif // DRAWBODY_H
