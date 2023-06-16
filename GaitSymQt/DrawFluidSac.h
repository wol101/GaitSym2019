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

#include "Drawable.h"

#include <QColor>

#include <memory>
#include <vector>

class FluidSac;
class FacetedObject;
class SimulationWidget;

class DrawFluidSac : public Drawable
{
public:
    DrawFluidSac();
    ~DrawFluidSac();

    virtual void initialise(SimulationWidget *simulationWidget);
    virtual void Draw();
    virtual std::string name();

    FluidSac *fluidSac() const;
    void setFluidSac(FluidSac *fluidSac);

    QColor fluidSacColour() const;
    void setFluidSacColour(const QColor &fluidSacColour);

    bool displayFluidSacForces() const;
    void setDisplayFluidSacForces(bool displayFluidSacForces);

    double fluidSacForceScale() const;
    void setFluidSacForceScale(double fluidSacForceScale);


private:
    FluidSac *m_fluidSac = nullptr;

    std::unique_ptr<FacetedObject> m_facetedObject;
    std::vector<std::unique_ptr<FacetedObject>> m_facetedObjectForceList;

    QColor m_fluidSacColour;
    QColor m_fluidSacForceColour;
    bool m_displayFluidSacForces;
    double m_fluidSacForceScale;
    double m_fluidSacForceRadius;
    size_t m_fluidSacForceSegments;
};

#endif // DRAWFLUIDSAC_H
