/*
 *  TwoPointStrap.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef TwoPointStrap_h
#define TwoPointStrap_h

#include "Strap.h"

class Body;
class Marker;

class TwoPointStrap:public Strap
{
public:

    TwoPointStrap();
    virtual ~TwoPointStrap();

    void SetOrigin(Body *body, const dVector3 point);
    void SetInsertion(Body *body, const dVector3 point);
    void SetOrigin(Body *body, const char *buf);
    void SetInsertion(Body *body, const char *buf);
    void SetOrigin(Marker *originMarker);
    void SetInsertion(Marker *insertionMarker);

    void GetOrigin(Body **body, double **origin) { *body = m_OriginBody; *origin = m_Origin; }
    void GetInsertion(Body **body, double **origin) { *body = m_InsertionBody; *origin = m_Insertion; }

    virtual void Calculate(double simulationTime);

    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);

    virtual std::set<Marker *> *updateDependentMarkers();

    virtual std::string *CreateFromAttributes();
    virtual void AppendToAttributes();

    Body *OriginBody() const;

    double *Origin();

    Body *InsertionBody() const;

    double *Insertion();

    Marker *originMarker() const;

    Marker *insertionMarker() const;

private:

    Body *m_OriginBody = nullptr;
    dVector3 m_Origin;
    Body *m_InsertionBody = nullptr;
    dVector3 m_Insertion;

    Marker *m_originMarker = nullptr;
    Marker *m_insertionMarker = nullptr;
};


#endif
