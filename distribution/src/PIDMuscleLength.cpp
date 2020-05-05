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
#include "GSUtil.h"

#include "pystring.h"

using namespace std::string_literals;

PIDMuscleLength::PIDMuscleLength()
{
}

void PIDMuscleLength::Initialise(double Kp, double Ki, double Kd)
{
    m_Kp = Kp;
    m_Ki = Ki;
    m_Kd = Kd;
    m_setpoint = 0;
    m_previous_error = DBL_MAX;
    m_error = 0;
    m_integral = 0;
    m_derivative = 0;
    m_output = 0;
    m_last_activation = 0;
    m_nomimal_length = 1;
    m_actual_position = 0;
    m_dt = 0;
}

void PIDMuscleLength::SetActivation(double activation, double duration)
{
    if (activation != m_setpoint) // reset the error values when the target value changes
    {
        m_setpoint = activation;
        m_previous_error = DBL_MAX;
        m_integral = 0;
    }
    m_actual_position = m_muscle->GetLength() / m_nomimal_length;
    m_dt = duration;

    // do the PID calculations
    m_error = m_setpoint - m_actual_position;
    if (m_previous_error == DBL_MAX) m_previous_error = m_error;
    m_integral = m_integral + (m_error * m_dt);
    m_derivative = (m_error - m_previous_error) / m_dt;
    m_output = (m_Kp * m_error) + (m_Ki * m_integral) + (m_Kd * m_derivative);
    m_previous_error = m_error;

    // now alter the activation based on the PID output
    activation = m_last_activation - m_output;
    if (activation < 0) activation = 0;
    else if (activation > 1) activation = 1;

    std::cerr << "PIDMuscleLength::SetActivation currently untested\n";
    // m_Muscle->SetActivation(activation, duration);
    m_last_activation = activation;
}

void PIDMuscleLength::SetMuscle(Muscle *muscle)
{
    m_muscle = muscle;
}

void PIDMuscleLength::SetNominalLength(double length)
{
    m_nomimal_length = length;
}

double PIDMuscleLength::GetActivation()
{
    return m_last_activation;
}

void PIDMuscleLength::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *PIDMuscleLength::createFromAttributes()
{
    if (Controller::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (findAttribute("Kp"s, &buf) == nullptr) return lastErrorPtr();
    double Kp = GSUtil::Double(buf);
    if (findAttribute("Ki"s, &buf) == nullptr) return lastErrorPtr();
    double Ki = GSUtil::Double(buf);
    if (findAttribute("Kd"s, &buf) == nullptr) return lastErrorPtr();
    double Kd = GSUtil::Double(buf);
    Initialise(Kp, Ki, Kd);
    if (findAttribute("NominalLength"s, &buf) == nullptr) return lastErrorPtr();
    SetNominalLength(GSUtil::Double(buf));
    if (findAttribute("MuscleID"s, &buf) == nullptr) return lastErrorPtr();
    auto muscleIter = simulation()->GetMuscleList()->find(buf);
    if (muscleIter != simulation()->GetMuscleList()->end()) this->SetMuscle(muscleIter->second.get());
    else
    {
        setLastError("Controller ID=\""s + name() +"\" MuscleID=\""s + buf + "\" not found"s);
        return lastErrorPtr();
    }
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void PIDMuscleLength::appendToAttributes()
{
    Controller::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "PIDErrorIn"s);
    setAttribute("Kp"s, *GSUtil::ToString(m_Kp, &buf));
    setAttribute("Ki"s, *GSUtil::ToString(m_Ki, &buf));
    setAttribute("Kd"s, *GSUtil::ToString(m_Kd, &buf));
    setAttribute("NominalLength"s, *GSUtil::ToString(m_nomimal_length, &buf));
    setAttribute("MuscleID"s, m_muscle->name());
}

