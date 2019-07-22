/*
 *  NPointStrap.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 27/10/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#ifndef NPointStrap_h
#define NPointStrap_h

#include "TwoPointStrap.h"

class Body;
class Marker;

class NPointStrap: public TwoPointStrap
{
public:

    NPointStrap();
    virtual ~NPointStrap();

    void SetViaPoints(std::vector<Body *> *bodyList, std::vector<double *> *pointList);
    void SetViaPoints(std::vector<Marker *> *viaPointMarkerList);

    std::vector<double *> *GetViaPoints() { return &m_ViaPointList; }
    std::vector<Body *> *GetViaPointBodies() { return &m_ViaBodyList; }

    virtual void Calculate(double simulationTime);

    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::set<Marker *> *updateDependentMarkers();

    virtual std::string *CreateFromAttributes();
    virtual void AppendToAttributes();

private:

    std::vector<Body *> m_ViaBodyList;
    std::vector<double *> m_ViaPointList;
    std::vector<Marker *> m_ViaPointMarkerList;
};


#endif
