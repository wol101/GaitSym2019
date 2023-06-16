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

    void SetVMax(double vMax);
    void SetF0(double f0);
    void SetK(double k);

    virtual double GetMetabolicPower();

    virtual void SetActivation();
    virtual double GetActivation();
    virtual double GetElasticEnergy();

    virtual std::string dumpToString();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    double forcePerUnitArea() const;
    void setForcePerUnitArea(double forcePerUnitArea);

    double vMaxFactor() const;
    void setVMaxFactor(double vMaxFactor);

    double pca() const;
    void setPca(double pca);

    double fibreLength() const;
    void setFibreLength(double fibreLength);

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
