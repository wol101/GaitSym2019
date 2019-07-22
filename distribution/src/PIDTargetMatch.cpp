/*
 *  PIDTargetMatch.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 3/4/2014.
 *  Copyright 2014 Bill Sellers. All rights reserved.
 *
 */

#include "ode/ode.h"

#include "PIDTargetMatch.h"
#include "Muscle.h"
#include "DataTarget.h"

PIDTargetMatch::PIDTargetMatch()
{
    m_Muscle = 0;
    m_DataTarget = 0;
    Kp = 0;
    Ki = 0;
    Kd = 0;
    previous_error = 0;
    error = 0;
    integral = 0;
    derivative = 0;
    output = 0;
    last_activation = 0;
    last_set_activation = 0;
}

// the activation value here is just a global gain modifier
// probably it should be set to 1 for normal use
// and 0 to deactivate the controller (although this will just leave the current value fixed)
void PIDTargetMatch::SetActivation(double activation, double duration)
{
    if (activation != last_set_activation) // reset the error values when the target value changes
    {
        last_set_activation = activation;
        previous_error = 0;
        integral = 0;
    }
    double dt = duration;

    // do the PID calculations
    error = m_DataTarget->GetError(0); // this assumes that the error is at index zero and it won't always be
    integral = integral + (error * dt);
    derivative = (error - previous_error) / dt;
    output = (Kp * error) + (Ki * integral) + (Kd * derivative);
    previous_error = error;

    // now alter the activation based on the PID output
    double muscleActivation = last_activation - activation * output;
    if (muscleActivation < 0) muscleActivation = 0;
    else if (muscleActivation > 1) muscleActivation = 1;

#ifdef LOCAL_DEBUG
    if (m_Name == "RightAdductorBrevisRightHipJointDataTargetController")
    {
        std::cerr << "error " << error << " integral " << integral << " derivative " << derivative << " output " << output <<
                     " muscleActivation " << muscleActivation << "\n";
    }
#endif

    std::cerr << "PIDTargetMatch::SetActivation currently untested\n";
    // m_Muscle->SetActivation(muscleActivation, duration);
    last_activation = muscleActivation;
}

