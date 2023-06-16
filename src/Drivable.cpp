/*
 *  Drivable.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 6/3/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include "Drivable.h"
#include "Driver.h"

Drivable::Drivable()
{
}

Drivable::~Drivable()
{
}

void Drivable::ReceiveData(double receivedData, int64_t receiveDataStepCount)
{
    if (receiveDataStepCount == m_receiveDataStepCount)
    {
        m_dataSum += receivedData;
    }
    else
    {
        m_receiveDataStepCount = receiveDataStepCount;
        m_dataSum = receivedData;
    }
}

double Drivable::dataSum() const
{
    return m_dataSum;
}

void Drivable::setDataSum(double dataSum)
{
    m_dataSum = dataSum;
}
