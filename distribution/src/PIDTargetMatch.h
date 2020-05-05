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

    void SetMuscle(Muscle *muscle);
    void Initialise(double Kp, double Ki, double Kd);
    void SetDataTarget(DataTarget *dataTarget);

    virtual void SetActivation(double activation, double duration);
    virtual double GetActivation();

    virtual void Update();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:
    Muscle *m_muscle = nullptr;
    DataTarget *m_dataTarget = nullptr;

    double m_Kp = 0;
    double m_Ki = 0;
    double m_Kd = 0;
    double m_previous_error = DBL_MAX;
    double m_error = 0;
    double m_integral = 0;
    double m_derivative = 0;
    double m_output = 0;
    double m_last_activation = 0;
    double m_last_set_activation = 0;
    double m_dt = 0;

};

#endif // PIDTARGETMATCH_H
