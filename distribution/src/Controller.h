/*
 *  Controller.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 6/3/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Drivable.h"
#include "Driver.h"

class Controller : public Driver, public Drivable
{
public:
    Controller();

    virtual std::string dumpToString();
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

};

#endif // CONTROLLER_H
