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
#include "GSUtil.h"

#include "pystring.h"

using namespace std::string_literals;

PIDTargetMatch::PIDTargetMatch()
{
}

void PIDTargetMatch::Initialise(double Kp, double Ki, double Kd)
{
    m_Kp = Kp;
    m_Ki = Ki;
    m_Kd = Kd;
    m_previous_error = DBL_MAX;
    m_error = 0;
    m_integral = 0;
    m_derivative = 0;
    m_output = 0;
    m_last_activation = 0;
    m_last_set_activation = 0;
}

// the activation value here is just a global gain modifier
// probably it should be set to 1 for normal use
// and 0 to deactivate the controller (although this will just leave the current value fixed)
void PIDTargetMatch::SetActivation(double activation, double duration)
{
    if (activation != m_last_set_activation) // reset the error values when the target value changes
    {
        m_last_set_activation = activation;
        m_previous_error = DBL_MAX;
        m_integral = 0;
    }
    m_dt = duration;

    // do the PID calculations
    m_error = m_dataTarget->GetError(0); // this assumes that the error is at index zero and it won't always be
    if (m_previous_error == DBL_MAX) m_previous_error = m_error;
    m_integral = m_integral + (m_error * m_dt);
    m_derivative = (m_error - m_previous_error) / m_dt;
    m_output = (m_Kp * m_error) + (m_Ki * m_integral) + (m_Kd * m_derivative);
    m_previous_error = m_error;

    // now alter the activation based on the PID output
    double muscleActivation = m_last_activation - activation * m_output;
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
    m_last_activation = muscleActivation;
 }

void PIDTargetMatch::SetDataTarget(DataTarget *dataTarget)
{
    m_dataTarget = dataTarget;
}

void PIDTargetMatch::SetMuscle(Muscle *muscle)
{
    m_muscle = muscle;
}

double PIDTargetMatch::GetActivation()
{
    return m_last_activation;
}

void PIDTargetMatch::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *PIDTargetMatch::createFromAttributes()
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
    if (findAttribute("DataTargetID"s, &buf) == nullptr) return lastErrorPtr();
    auto dataTargetIter = simulation()->GetDataTargetList()->find(buf);
    if (dataTargetIter != simulation()->GetDataTargetList()->end()) this->SetDataTarget(dataTargetIter->second.get());
    else
    {
        setLastError("Controller ID=\""s + name() +"\" DataTargetID=\""s + buf + "\" not found"s);
        return lastErrorPtr();
    }
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
void PIDTargetMatch::appendToAttributes()
{
    Controller::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "PIDErrorIn"s);
    setAttribute("Kp"s, *GSUtil::ToString(m_Kp, &buf));
    setAttribute("Ki"s, *GSUtil::ToString(m_Ki, &buf));
    setAttribute("Kd"s, *GSUtil::ToString(m_Kd, &buf));
    setAttribute("DataTargetID"s, m_dataTarget->name());
    setAttribute("MuscleID"s, m_muscle->name());
}


