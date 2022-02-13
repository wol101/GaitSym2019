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

#include "Drawable.h"

#include <memory>

class Marker;
class SimulationWidget;

class DrawMarker : public Drawable
{
public:
    DrawMarker();
    virtual ~DrawMarker();

    virtual void initialise(SimulationWidget *simulationWidget);
    virtual void Draw();
    virtual std::string name();

    void updateEntityPose();

    Marker *marker() const;
    void setMarker(Marker *marker);

private:
    Marker *m_marker = nullptr;
    std::unique_ptr<FacetedObject> m_facetedObject;
};

#endif // DRAWMARKER_H
