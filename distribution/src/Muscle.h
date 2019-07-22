/*
 *  Muscle.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef Muscle_h
#define Muscle_h

#include <vector>
#include <string>

#include "Drivable.h"
#include "Strap.h"
#include "Simulation.h"

class Muscle: public Drivable
{
public:

    Muscle();
    virtual ~Muscle();

    double GetLength();
    double GetVelocity();
    double GetTension();
    double GetPower();

    void CalculateStrap(double simulationTime);

    virtual void SetActivation(double activation, double  duration) = 0;
    virtual double GetActivation() = 0;
    virtual double GetMetabolicPower() = 0;
    virtual double GetElasticEnergy() = 0;

    std::vector<PointForce *> *GetPointForceList();

    Strap *GetStrap();
    void SetStrap(Strap *strap);

    virtual int SanityCheck(Muscle *otherMuscle, Simulation::AxisType axis, const std::string &sanityCheckLeft,
                            const std::string &sanityCheckRight);
    virtual void LateInitialisation();
    virtual std::string *CreateFromAttributes();
    virtual void SaveToAttributes();
    virtual void AppendToAttributes();

private:

    Strap *m_Strap = nullptr;
};

#endif

