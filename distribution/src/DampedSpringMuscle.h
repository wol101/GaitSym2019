/*
 *  DampedSpringMuscle.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// DampedSpringMuscle - implementation of a damped spring strap force

#ifndef DampedSpringMuscle_h
#define DampedSpringMuscle_h

#include "Muscle.h"

class Strap;

class DampedSpringMuscle : public Muscle
{
public:

    DampedSpringMuscle();

    void SetDamping(double d) { m_Damping = d; } // value is in N/(m2 s) (Stress per Strain rate)
    void SetSpringConstant(double k) { m_SpringConstant = k; } // value is in N/m2 (this is the Young's Modulus)
    void SetUnloadedLength(double l) { m_UnloadedLength = l; } // value is in m
    void SetArea(double a) { m_Area = a; } // value is in m2
    void SetBreakingStrain(double breakingStrain) { m_BreakingStrain = breakingStrain; } // value is strain
    double GetDamping() { return m_Damping; } // value is in N/m2
    double GetSpringConstant() { return m_SpringConstant; } // value is in N/m2
    double GetUnloadedLength() { return m_UnloadedLength; } // value is in m
    double GetArea() { return m_Area; } // value is in m2
    double GetBreakingStrain() { return m_BreakingStrain; } // value is strain
    double GetElasticEnergy();

    virtual void SetActivation();
    virtual double GetActivation() { return m_Activation; }
    virtual double GetMetabolicPower() { return 0; }

    bool ShouldBreak();

    virtual std::string dumpToString();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    double m_Damping = 0;
    double m_SpringConstant = 0;
    double m_UnloadedLength = 0;
    double m_Area = 1;
    double m_BreakingStrain = 0;
    double m_Activation = 0;
};








#endif
