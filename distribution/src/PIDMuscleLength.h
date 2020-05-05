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

    void SetMuscle(Muscle *muscle);
    void Initialise(double Kp, double Ki, double Kd);

    void SetNominalLength(double length);

    virtual void SetActivation(double activation, double duration);
    virtual double GetActivation();

    virtual void Update();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:
    Muscle *m_muscle = nullptr;
    double m_Kp = 0;
    double m_Ki = 0;
    double m_Kd = 0;
    double m_setpoint = 0;
    double m_previous_error = DBL_MAX;
    double m_error = 0;
    double m_integral = 0;
    double m_derivative = 0;
    double m_output = 0;
    double m_last_activation = 0;
    double m_nomimal_length = 1;
    double m_actual_position = 0;
    double m_dt = 0;
};

#endif // PIDMUSCLELENGTH_H
