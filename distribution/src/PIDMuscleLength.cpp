/*
 *  PIDMuscleLength.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 6/3/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include "ode/ode.h"

#include "PIDMuscleLength.h"
#include "Muscle.h"

PIDMuscleLength::PIDMuscleLength()
{
    m_Muscle = 0;
    Kp = 0;
    Ki = 0;
    Kd = 0;
    setpoint = 0;
    previous_error = 0;
    error = 0;
    integral = 0;
    derivative = 0;
    output = 0;
    last_activation = 0;
    nomimal_length = 1;
}

void PIDMuscleLength::SetActivation(double activation, double duration)
{
    if (activation != setpoint) // reset the error values when the target value changes
    {
        setpoint = activation;
        previous_error = 0;
        integral = 0;
    }
    double actual_position = m_Muscle->GetLength() / nomimal_length;
    double dt = duration;

    // do the PID calculations
    error = setpoint - actual_position;
    integral = integral + (error * dt);
    derivative = (error - previous_error) / dt;
    output = (Kp * error) + (Ki * integral) + (Kd * derivative);
    previous_error = error;

    // now alter the activation based on the PID output
    activation = last_activation - output;
    if (activation < 0) activation = 0;
    else if (activation > 1) activation = 1;

    std::cerr << "PIDMuscleLength::SetActivation currently untested\n";
    // m_Muscle->SetActivation(activation, duration);
    last_activation = activation;
}

