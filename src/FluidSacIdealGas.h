/*
 *  FluidSacIdealGas.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 02/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef FLUIDSACIDEALGAS_H
#define FLUIDSACIDEALGAS_H

#include "FluidSac.h"

class FluidSacIdealGas : public FluidSac
{
public:
    FluidSacIdealGas();

    virtual void calculatePressure();
    virtual void LateInitialisation();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    double temperature() const;
    void setTemperature(double temperature);

    double amountOfSubstance() const;
    void setAmountOfSubstance(double amountOfSubstance);

    double externalPressure() const;
    void setExternalPressure(double externalPressure);

    double R() const;
    void setR(double R);

private:
    double m_R = { 8.314 };
    double m_amountOfSubstance = { 0 };
    double m_temperature = { 273.15 + 20 }; // 20 degrees C in Kelvin (NTP)
    double m_externalPressure = { 101.325e3 }; // NTP value
};

#endif // FLUIDSACIDEALGAS_H
