/*
 *  FixedDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat Jun 21 2014.
 *  Copyright (c) 2014 Bill Sellers. All rights reserved.
 *
 *  Outputs a fixed value
 *
 */

#ifndef FIXEDDRIVER_H
#define FIXEDDRIVER_H

#include "Driver.h"

class FixedDriver : public Driver
{
public:
    FixedDriver();

    virtual void Update();

    void MultiplyValue(double mod);
    void AddValue(double mod);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:
};

#endif // FIXEDDRIVER_H
