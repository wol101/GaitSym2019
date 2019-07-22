/*
 *  PIDMuscleLength.h
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

class PIDMuscleLength : public Controller
{
public:
    PIDMuscleLength();

    void SetMuscle(Muscle *muscle) { m_Muscle = muscle; }
    void SetPID(double P, double I, double D) { Kp = P; Ki = I; Kd = D; }

    void SetNominalLength(double length) { nomimal_length = length; }

    virtual void SetActivation(double activation, double duration);
    virtual double GetActivation() { return last_activation; }

    virtual double GetValue(double /* time */) { return 0; } // dummy value for now

private:
    Muscle *m_Muscle;
    double Kp;
    double Ki;
    double Kd;
    double setpoint;
    double previous_error;
    double error;
    double integral;
    double derivative;
    double output;
    double last_activation;
    double nomimal_length;
};

#endif // PIDMUSCLELENGTH_H
