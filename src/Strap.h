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

    double GetLength() const;
    double GetVelocity() const;

    void SetTension(double tension);
    double GetTension() const;

    void GetTorque(const Marker &marker, pgd::Vector3 *worldTorque, pgd::Vector3 *markerTorque, pgd::Vector3 *worldMomentArm, pgd::Vector3 *markerMomentArm);

    std::vector<std::unique_ptr<PointForce >> *GetPointForceList();

    virtual void Calculate() = 0;

//    virtual int SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight) = 0;

    virtual std::string dumpToString();

    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

    double Length() const;
    void setLength(double Length);

    double Velocity() const;
    void setVelocity(double Velocity);

    double Tension() const;
    void setTension(double Tension);

    std::vector<Marker *> torqueMarkerList() const;
    void setTorqueMarkerList(const std::vector<Marker *> &torqueMarkerList);

private:

    double m_tension = -1;
    double m_velocity = -1;
    double m_length = -1;

    std::vector<std::unique_ptr<PointForce >> m_pointForceList;
    std::vector<Marker *> m_torqueMarkerList;

};

#endif

