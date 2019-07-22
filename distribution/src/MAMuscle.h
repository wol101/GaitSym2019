/*
 *  MAMuscle.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// MAMuscle - implementation of an Minetti & Alexander style
// muscle based on the StrapForce class

// Minetti & Alexander, J. theor Biol (1997) 186, 467-476

// Added extra terms to allow a parallel spring element

#ifndef MAMuscle_h
#define MAMuscle_h

#include "Muscle.h"

class Strap;

class MAMuscle : public Muscle
{
public:

    MAMuscle();
    virtual ~MAMuscle();

    void SetVMax(double vMax) { m_VMax = vMax; }
    void SetF0(double f0) { m_F0 = f0; }
    void SetK(double k) { m_K = k; }

    virtual double GetMetabolicPower();

    virtual void SetActivation(double activation, double /* duration */) { SetAlpha(activation); }
    virtual double GetActivation() { return m_Alpha; }
    virtual double GetElasticEnergy() { return 0; }

    virtual void Dump();

    virtual std::string *CreateFromAttributes();
    virtual void AppendToAttributes();

private:

    void SetAlpha(double alpha);

    double m_VMax = 0;
    double m_F0 = 0;
    double m_K = 0;
    double m_Alpha = 0;

    // these values are only used for loading and saving
    double m_forcePerUnitArea = 0;
    double m_vMaxFactor = 0;
    double m_pca = 0;
    double m_fibreLength = 0;

};








#endif // MAMuscle_h
