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

#include "Strap.h"
#include "PGDMath.h"

class Body;
class Marker;

class NPointStrap: public Strap
{
public:

    NPointStrap();

//    void SetOrigin(Body *body, const dVector3 point);
//    void SetInsertion(Body *body, const dVector3 point);
    void SetOrigin(Marker *originMarker);
    void SetInsertion(Marker *insertionMarker);

//    void SetViaPoints(std::vector<Body *> *bodyList, std::vector<pgd::Vector3> *pointList);
    void SetViaPoints(std::vector<Marker *> *viaPointMarkerList);

//    void GetOrigin(const Body **body, dVector3 origin) const;
//    void GetInsertion(const Body **body, dVector3 insertion) const;

    const std::vector<pgd::Vector3> *GetViaPoints() const;
    const std::vector<Body *> *GetViaPointBodies() const;
    const std::vector<Marker *> *GetViaPointMarkers() const;

    Marker *GetOriginMarker() const;
    Marker *GetInsertionMarker() const;


    virtual void Calculate();

//    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

//    Body *m_originBody = nullptr;
//    dVector3 m_origin;
//    Body *m_insertionBody = nullptr;
//    dVector3 m_insertion;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;

    std::vector<Body *> m_ViaBodyList;
    std::vector<pgd::Vector3> m_ViaPointList;
    std::vector<Marker *> m_ViaPointMarkerList;
};


#endif
