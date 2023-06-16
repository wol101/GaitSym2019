/*
 *  FluidSacIncompressible.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 02/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef FLUIDSACINCOMPRESSIBLE_H
#define FLUIDSACINCOMPRESSIBLE_H

#include "FluidSac.h"

class FluidSacIncompressible : public FluidSac
{
public:
    FluidSacIncompressible();

    virtual void calculatePressure();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    double bulkModulus() const;
    void setBulkModulus(double bulkModulus);

    double fluidVolume() const;
    void setFluidVolume(double fluidVolume);

    double startingPressure() const;
    void setStartingPressure(double startingPressure);

    double bulkModulusDamping() const;
    void setBulkModulusDamping(double newBulkModulusDamping);

private:
    double m_fluidVolume = {0};
    double m_startingPressure = {0};
    double m_bulkModulus = {0};
    double m_bulkModulusDamping = {0};
};

#endif // FLUIDSACINCOMPRESSIBLE_H
