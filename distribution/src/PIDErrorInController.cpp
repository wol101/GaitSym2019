/*
 *  PIDErrorInController.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#include "PIDErrorInController.h"

#define RANGE(x, l, h) if (x < (l)) x = (l); if (x > (h)) x = (h);

PIDErrorInController::PIDErrorInController()
{
    // these are the standard PID controller values
    m_Kp = 0;
    m_Ki = 0;
    m_Kd = 0;
    m_previous_error = 0;
    m_error = 0;
    m_integral = 0;
    m_derivative = 0;
    m_output = 0;
    m_dt = 0;
//    m_outputIsDelta = false;
}

void PIDErrorInController::Initialise(double Kp, double Ki, double Kd)
{
    m_Kp = Kp;
    m_Ki = Ki;
    m_Kd = Kd;
    m_previous_error = 0;
    m_error = 0;
    m_integral = 0;
    m_derivative = 0;
    m_output = 0;
    setLastTime(0);
    setLastValue(0);
    m_dt = 0;
}

double PIDErrorInController::GetValue(double time)
{
    if (time == LastTime()) return LastValue();
    m_dt = time - LastTime();
    setLastTime(time);

    // in this driver, the error is driven by the upstream driver
    SumDrivers(time);
    m_error = GetCurrentDriverSum();

    // do the PID calculations
    m_integral = m_integral + (m_error * m_dt);
    m_derivative = (m_error - m_previous_error) / m_dt;
    m_output = (m_Kp * m_error) + (m_Ki * m_integral) + (m_Kd * m_derivative);
    m_previous_error = m_error;

    // now alter the output based on the PID output
    double output = LastValue() - m_output;
    RANGE(output, MinValue(), MaxValue());
    setLastValue(output);
    return output;
}

//void PIDErrorInController::setOutputIsDelta(bool outputIsDelta)
//{
//    m_outputIsDelta = outputIsDelta;
//}
