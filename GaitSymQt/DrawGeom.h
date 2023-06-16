/*
 *  DrawGeom.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 11/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRAWGEOM_H
#define DRAWGEOM_H

#include <Drawable.h>

#include <QColor>

#include <memory>

class Geom;
class FacetedObject;
class SimulationWidget;

class DrawGeom : public Drawable
{
public:
    DrawGeom();
    virtual ~DrawGeom();

    virtual void initialise(SimulationWidget *simulationWidget);
    virtual void Draw();
    virtual std::string name();

    void updateEntityPose();

    Geom *geom() const;
    void setGeom(Geom *geom);

private:
    Geom *m_geom = nullptr;

    std::unique_ptr<FacetedObject> m_facetedObject;

    QColor m_geomColor1;
    QColor m_geomColor2;
    size_t m_geomSegments = 0;
    double m_geomSize1 = 0;
    double m_geomSize2 = 0;

};

#endif // DRAWGEOM_H
