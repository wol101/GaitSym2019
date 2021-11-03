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

#include "Drivable.h"
#include "Strap.h"
#include "Simulation.h"
#include "SmartEnum.h"

#include <string>

class Muscle: public Drivable, public NamedObject
{
public:

    Muscle();
    virtual ~Muscle();

    double GetLength() const;
    double GetVelocity() const;
    double GetTension() const;
    double GetPower() const;

    void CalculateStrap();

    SMART_ENUM(StrapColourControl, strapColourControlStrings, strapColourControlCount, fixedColour, activationMap, strainMap, forceMap);
//    enum StrapColourControl { fixedColour, activationMap, strainMap, forceMap };

    virtual void SetActivation() = 0;
    virtual double GetActivation() = 0;
    virtual double GetMetabolicPower() = 0;
    virtual double GetElasticEnergy() = 0;

    std::vector<std::unique_ptr<PointForce >> *GetPointForceList() const;

    Strap *GetStrap() const;
    void SetStrap(Strap *strap);

//    virtual int SanityCheck(Muscle *otherMuscle, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight);
    virtual void LateInitialisation();
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

    StrapColourControl strapColourControl() const;
    void setStrapColourControl(const Muscle::StrapColourControl &strapColourControl);

private:

    Strap *m_Strap = nullptr;
    enum StrapColourControl m_strapColourControl = fixedColour;
};

#endif

