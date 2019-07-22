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



using namespace std::string_literals;

FixedDriver::FixedDriver()
{
    mValue = 0;
}

FixedDriver::~FixedDriver()
{
}


double FixedDriver::GetValue(double time)
{
    if (time == LastTime()) return LastValue();
    setLastTime(time);
    setLastValue(Clamp(mValue));
    return LastValue();
}

void FixedDriver::SetValue(double value)
{
    mValue = value;
}

void FixedDriver::MultiplyValue(double mod)
{
    mValue *= mod;
}

void FixedDriver::AddValue(double mod)
{
    mValue += mod;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *FixedDriver::CreateFromAttributes()
{
    if (Driver::CreateFromAttributes()) return lastErrorPtr();

    std::string buf;
    if (GetAttribute("Value"s, &buf) == nullptr) return lastErrorPtr();
    this->SetValue(GSUtil::Double(buf));

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void FixedDriver::AppendToAttributes()
{
    Driver::AppendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Fixed"s);
    setAttribute("Value"s, *GSUtil::ToString(mValue, &buf));
}

