/*
 *  Drivable.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 6/3/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#ifndef DRIVABLE_H
#define DRIVABLE_H

#include <vector>

#include "NamedObject.h"

class Driver;

class Drivable : public NamedObject
{
public:
    Drivable();

    void AddDriver(Driver *driver) { m_driverList.push_back(driver); }
    double GetCurrentDriverSum() { return m_currentDriverSum; }
    void SetCurrentDriverSum(double currentDriverSum) { m_currentDriverSum = currentDriverSum; }
    double SumDrivers(double time);

private:
    std::vector<Driver *> m_driverList;
    double m_currentDriverSum;
};

#endif // DRIVABLE_H
