/*
 *  Controller.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 6/3/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include "Controller.h"
#include "Simulation.h"

#include <fstream>
#include <sstream>

Controller::Controller()
{
}

std::string Controller::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tValue\tDriverSum\n";
    }
    ss << simulation()->GetTime() << "\t" << value() << "\t" << dataSum() << "\n";
    return ss.str();
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Controller::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();
    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Controller::saveToAttributes()
{
    this->setTag("CONTROLLER"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void Controller::appendToAttributes()
{
    Driver::appendToAttributes();
}
