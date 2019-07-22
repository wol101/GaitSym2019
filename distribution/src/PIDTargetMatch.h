/*
 *  PIDTargetMatch.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 3/5/2014.
 *  Copyright 2014 Bill Sellers. All rights reserved.
 *
 */

#ifndef PIDTARGETMATCH_H
#define PIDTARGETMATCH_H

#include "Controller.h"

class Muscle;
class DataTarget;

class PIDTargetMatch : public Controller
{
public:
    PIDTargetMatch();

    void SetMuscle(Muscle *muscle) { m_Muscle = muscle; }
    void SetPID(double P, double I, double D) { Kp = P; Ki = I; Kd = D; }
    void SetDataTarget(DataTarget *target) { m_DataTarget = target; }

    virtual void SetActivation(double activation, double duration);
    virtual double GetActivation() { return last_activation; }

    virtual double GetValue(double /* time */) { return 0; } // dummy value for now

private:
    Muscle *m_Muscle;
    DataTarget *m_DataTarget;

    double Kp;
    double Ki;
    double Kd;
    double previous_error;
    double error;
    double integral;
    double derivative;
    double output;
    double last_activation;
    double last_set_activation;
};

#endif // PIDTARGETMATCH_H
