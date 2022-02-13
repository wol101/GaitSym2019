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

#include <Drawable.h>

#include <QColor>

#include <memory>

class Joint;
class FacetedObject;
class SimulationWidget;
class TexturedQuad;

class DrawJoint : public Drawable
{
public:
    DrawJoint();
    virtual ~DrawJoint();

    virtual void initialise(SimulationWidget *simulationWidget);
    virtual void Draw();
    virtual std::string name();

    void updateEntityPose();

    Joint *joint() const;
    void setJoint(Joint *joint);

private:
    Joint *m_joint = nullptr;

    std::unique_ptr<FacetedObject> m_facetedObject1;
    std::unique_ptr<FacetedObject> m_facetedObject2;
    std::unique_ptr<FacetedObject> m_facetedObject3;
//    std::unique_ptr<TexturedQuad> m_texturedQuad;

    double m_jointAxisSize;
    QColor m_jointColor;
    size_t m_jointSegments;

};

#endif // DRAWJOINT_H
