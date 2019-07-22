/*
 *  Strap.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef Strap_h
#define Strap_h

#include "NamedObject.h"
#include "Simulation.h"
#include <vector>
#include <set>

class Body;
class FacetedObject;
class Marker;

struct PointForce
{
    Body *body;         // this is the body the force is applied to
    dVector3 point;     // this is the position of action in world coordinates
    dVector3 vector;    // this is the direction of action (magnitude acts as a scaling factor)
};

class Strap: public NamedObject
{
public:

    Strap();
    virtual ~Strap();

    double GetLength() { return m_Length; }
    double GetVelocity() { return m_Velocity; }

    void SetTension(double tension) { m_Tension = tension; }
    double GetTension() { return m_Tension; }

    std::vector<PointForce *> *GetPointForceList() { return &m_PointForceList; }

    virtual void Calculate(double simulationTime) = 0;

    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight) = 0;

    virtual void Dump();

    virtual std::string *CreateFromAttributes();
    virtual void SaveToAttributes();
    virtual void AppendToAttributes();

    std::vector<PointForce *> *PointForceList();
    std::set<Marker *> *dependentMarkers();
    virtual std::set<Marker *> *updateDependentMarkers() = 0;

    double Length() const;
    void setLength(double length, double simulationTime);

    double Velocity() const;

    double Tension() const;
    void setTension(double Tension);


private:

    double m_LastTime = -1;
    double m_Tension = -1;
    double m_Velocity = -1;
    double m_Length = -1;
    bool m_saveLengthFlag = false;

    std::vector<PointForce *> m_PointForceList;
    std::set<Marker *> m_dependentMarkers;

};

#endif

