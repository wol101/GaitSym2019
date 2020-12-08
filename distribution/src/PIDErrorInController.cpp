/*
 *  PIDErrorInController.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#include "PIDErrorInController.h"
#include "GSUtil.h"
#include "Simulation.h"

#include "pystring.h"

using namespace std::string_literals;

PIDErrorInController::PIDErrorInController()
{
}

void PIDErrorInController::Initialise(double Kp, double Ki, double Kd)
{
    m_Kp = Kp;
    m_Ki = Ki;
    m_Kd = Kd;
    m_previous_error = DBL_MAX;
    m_error = 0;
    m_integral = 0;
    m_derivative = 0;
    m_output = 0;
    m_dt = 0;
}

void PIDErrorInController::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());

    m_dt = simulation()->GetTimeIncrement();

    // in this driver, the error is driven by the upstream driver
    m_error = dataSum();
    if (m_previous_error == DBL_MAX) m_previous_error = m_error;

    // do the PID calculations
    m_integral = m_integral + (m_error * m_dt);
    m_derivative = (m_error - m_previous_error) / m_dt;
    m_output = (m_Kp * m_error) + (m_Ki * m_integral) + (m_Kd * m_derivative);
    m_previous_error = m_error;

    // now set the output based on the PID output
    // note that we limit the value to the range
    setValue(Clamp(m_output));
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *PIDErrorInController::createFromAttributes()
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
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void PIDErrorInController::appendToAttributes()
{
    Controller::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "PIDErrorIn"s);
    setAttribute("Kp"s, *GSUtil::ToString(m_Kp, &buf));
    setAttribute("Ki"s, *GSUtil::ToString(m_Ki, &buf));
    setAttribute("Kd"s, *GSUtil::ToString(m_Kd, &buf));
}

std::string PIDErrorInController::dumpToString()
{
    std::string s;
    if (firstDump())
    {
        setFirstDump(false);
        s = dumpHelper({"Time", "Kp"s, "Ki"s, "Kd"s, "previous_error"s, "error"s, "integral"s, "derivative"s, "output"s, "dt"s, "value"s});
    }
    s += dumpHelper({simulation()->GetTime(), m_Kp, m_Ki, m_Kd, m_previous_error, m_error, m_integral, m_derivative, m_output, m_dt, value()});
    return s;
}

