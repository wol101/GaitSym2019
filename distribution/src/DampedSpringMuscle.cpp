/*
 *  DampedSpringMuscle.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// DampedSpringMuscle - implementation of a damped spring strap force

#include "Strap.h"
#include "DampedSpringMuscle.h"
#include "Simulation.h"
#include "GSUtil.h"

#include <sstream>

using namespace std::string_literals;

// constructor

DampedSpringMuscle::DampedSpringMuscle(): Muscle()
{
}

double DampedSpringMuscle::GetElasticEnergy()
{
    double delLen = GetStrap()->GetLength() - m_UnloadedLength;
    if (delLen < 0) return 0;

    // difference between these two values is the amount of energy lost by damping
    // std::cerr << 0.5 * GetStrap()->GetTension() * delLen << "\n";
    // std::cerr << 0.5 * m_SpringConstant * m_Area * delLen * delLen / m_UnloadedLength << "\n";

    return 0.5 * m_SpringConstant * m_Area * delLen * delLen / m_UnloadedLength;
}


// update the tension depending on length and velocity
// activation is used as a linear multiplier
void DampedSpringMuscle::SetActivation()
{
    m_Activation = dataSum();

    // calculate strain
    double elasticStrain = (GetStrap()->GetLength() - m_UnloadedLength) / m_UnloadedLength;

    // calculate stress
    double elasticStress = elasticStrain * m_SpringConstant;
    double tension;
    if (elasticStress <= 0) // if not stretching the spring then set tension to zero
    {
        tension = 0;
    }
    else
    {

        // calculate damping (+ve when lengthening)
        double relativeVelocity = GetStrap()->GetVelocity() / m_UnloadedLength;
        double dampingStress = relativeVelocity * m_Damping;

        // now calculate tension
        // NB. tension is negative when muscle shortening
        tension = (elasticStress + dampingStress) * m_Area * m_Activation;

        // stop any pushing
        if (tension < 0) tension = 0;
    }
    GetStrap()->SetTension(tension);
}

bool DampedSpringMuscle::ShouldBreak()
{
    if (m_BreakingStrain <= 0) return false;
    double elasticStrain = (GetStrap()->GetLength() - m_UnloadedLength) / m_UnloadedLength;
    if (elasticStrain > m_BreakingStrain)
    {
        std::cerr << "DampedSpringMuscle::ShouldBreak returns true\n";
        return true;
    }
    return false;
}

std::string DampedSpringMuscle::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tact\ttension\tlength\tvelocity\tPMECH\n";
    }
    ss << simulation()->GetTime() << "\t" << m_Activation <<
          "\t" << GetStrap()->GetTension() << "\t" << GetStrap()->GetLength() << "\t" << GetStrap()->GetVelocity() <<
          "\t" << GetStrap()->GetVelocity() * GetStrap()->GetTension() <<
          "\n";
    return ss.str();
}

std::string *DampedSpringMuscle::createFromAttributes()
{
    if (Muscle::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (findAttribute("UnloadedLength"s, &buf) == nullptr) return lastErrorPtr();
    this->SetUnloadedLength(GSUtil::Double(buf.c_str()));
    if (findAttribute("SpringConstant"s, &buf) == nullptr) return lastErrorPtr();
    this->SetSpringConstant(GSUtil::Double(buf.c_str()));
    if (findAttribute("Area"s, &buf) == nullptr) return lastErrorPtr();
    this->SetArea(GSUtil::Double(buf.c_str()));
    if (findAttribute("Damping"s, &buf) == nullptr) return lastErrorPtr();
    this->SetDamping(GSUtil::Double(buf.c_str()));
    if (findAttribute("BreakingStrain"s, &buf) == nullptr) return lastErrorPtr();
    this->SetBreakingStrain(GSUtil::Double(buf.c_str()));
    return nullptr;
}

 void DampedSpringMuscle::appendToAttributes()
{
     Muscle::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "DampedSpring"s);
    setAttribute("UnloadedLength"s, *GSUtil::ToString(m_UnloadedLength, &buf));
    setAttribute("SpringConstant"s, *GSUtil::ToString(m_SpringConstant, &buf));
    setAttribute("Area"s, *GSUtil::ToString(m_Area, &buf));
    setAttribute("Damping"s, *GSUtil::ToString(m_Damping, &buf));
    setAttribute("BreakingStrain"s, *GSUtil::ToString(m_BreakingStrain, &buf));
}

