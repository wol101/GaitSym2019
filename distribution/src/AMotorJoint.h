/*
 *  AMotorJoint.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 07/01/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#ifndef AMOTORJOINT_H
#define AMOTORJOINT_H

#include "Joint.h"

class AMotorJoint: public Joint
{
public:

    AMotorJoint(dWorldID worldID);

    void SetStops(double low, double high);
    void SetAxis(double x, double y, double z, int axisMode);
    void SetAxis(const char *buf);
    void SetTargetVelocity(double targetVelocity);
    void SetMaxTorque(double maximumTorque);

    double GetAngle();
    double GetAngleRate();

    virtual void Update();
    virtual void Dump();

    virtual std::string *XMLLoad(rapidxml::xml_node<char> * /* node */) { return nullptr; }
    virtual rapidxml::xml_node<char> *XMLSave(rapidxml::xml_node<char> * /* parent */) { return nullptr; }

private:

    void SetAngle();
};


#endif // AMOTORJOINT_H
