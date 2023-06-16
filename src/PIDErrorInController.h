/*
 *  PIDErrorInController.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "Controller.h"

#include <cfloat>

class PIDErrorInController : public Controller
{
public:
    PIDErrorInController();

    void Initialise(double Kp, double Ki, double Kd);

    virtual void Update();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    virtual std::string dumpToString();

private:

    double m_Kp = 0;
    double m_Ki = 0;
    double m_Kd = 0;
    double m_previous_error = DBL_MAX;
    double m_error = 0;
    double m_integral = 0;
    double m_derivative = 0;
    double m_output = 0;
    double m_dt = 0;
};

#endif // PIDCONTROLLER_H
