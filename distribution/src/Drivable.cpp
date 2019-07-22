/*
 *  Drivable.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 6/3/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include <vector>
#include "ode/ode.h"

#include "Driver.h"
#include "Drivable.h"

Drivable::Drivable()
{
    m_currentDriverSum = 0;
}

double Drivable::SumDrivers(double time)
{
    m_currentDriverSum = 0;
    for (std::vector<Driver *>::const_iterator iter1 = m_driverList.begin(); iter1 != m_driverList.end(); iter1++)
    {
        m_currentDriverSum += (*iter1)->GetValue(time);
    }
    return m_currentDriverSum;
}
