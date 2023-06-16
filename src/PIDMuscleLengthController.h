/*
 *  PIDMuscleLengthController.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 6/3/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */
#ifndef PIDMUSCLELENGTH_H
#define PIDMUSCLELENGTH_H

#include "Controller.h"

class Muscle;

class PIDMuscleLengthController : public Controller
{
public:
    PIDMuscleLengthController();

    void Initialise(double Kp, double Ki, double Kd);

    virtual void Update();

    Muscle *muscle();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    virtual std::string dumpToString();

private:
    double m_setpoint = 0;
    double m_Kp = 0;
    double m_Ki = 0;
    double m_Kd = 0;
    double m_previous_error = DBL_MAX;
    double m_error = 0;
    double m_integral = 0;
    double m_derivative = 0;
    double m_output = 0;
    double m_dt = 0;
    double m_current_length = 0;
};

#endif // PIDMUSCLELENGTH_H
