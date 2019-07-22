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

class PIDErrorInController : public Controller
{
public:
    PIDErrorInController();

    void Initialise(double Kp, double Ki, double Kd);

    virtual double GetValue(double time);

//    void setOutputIsDelta(bool outputIsDelta);

private:

    double m_Kp;
    double m_Ki;
    double m_Kd;
    double m_previous_error;
    double m_error;
    double m_integral;
    double m_derivative;
    double m_output;
    double m_dt;
//    bool m_outputIsDelta;

};

#endif // PIDCONTROLLER_H
