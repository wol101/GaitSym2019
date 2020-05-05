/*
 *  FixedDriver.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat Jun 21 2014.
 *  Copyright (c) 2014 Bill Sellers. All rights reserved.
 *
 *  Outputs a fixed value
 *
 */

#include "FixedDriver.h"
#include "GSUtil.h"
#include "Simulation.h"

#include <cassert>

using namespace std::string_literals;

FixedDriver::FixedDriver()
{
}

void FixedDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());
}

void FixedDriver::MultiplyValue(double mod)
{
    setValue(value() * mod);
}

void FixedDriver::AddValue(double mod)
{
    setValue(value() + mod);
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *FixedDriver::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    if (findAttribute("Value"s, &buf) == nullptr) return lastErrorPtr();
    this->setValue(GSUtil::Double(buf));

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void FixedDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Fixed"s);
    setAttribute("Value"s, *GSUtil::ToString(value(), &buf));
}

