/*
 *  DampedSpringMuscle.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// DampedSpringMuscle - implementation of a damped spring strap force

#include "ode/ode.h"


#include "Strap.h"
#include "DampedSpringMuscle.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "TwoPointStrap.h"
#include "NPointStrap.h"
#include "CylinderWrapStrap.h"
#include "TwoCylinderWrapStrap.h"

using namespace std::string_literals;

// constructor

DampedSpringMuscle::DampedSpringMuscle(): Muscle()
{
    m_Damping = 0;
    m_SpringConstant = 0;
    m_UnloadedLength = 0;
    m_Activation = 0;
    m_Area = 1;
    m_BreakingStrain = 0;
}

// destructor
DampedSpringMuscle::~DampedSpringMuscle()
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
void DampedSpringMuscle::SetActivation(double activation, double /* duration */)
{
    m_Activation = activation;

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

void DampedSpringMuscle::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == nullptr)
        {
            if (GetName().size() == 0) std::cerr << "NamedObject::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tact\ttension\tlength\tvelocity\tPMECH\n";
        }
    }


    if (dumpStream())
    {
        *dumpStream() << simulation()->GetTime() << "\t" << m_Activation <<
                "\t" << GetStrap()->GetTension() << "\t" << GetStrap()->GetLength() << "\t" << GetStrap()->GetVelocity() <<
                "\t" << GetStrap()->GetVelocity() * GetStrap()->GetTension() <<
                "\n";
    }
}

std::string *DampedSpringMuscle::CreateFromAttributes()
{
    if (Muscle::CreateFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (GetAttribute("UnloadedLength"s, &buf) == nullptr) return lastErrorPtr();
    this->SetUnloadedLength(GSUtil::Double(buf.c_str()));
    if (GetAttribute("SpringConstant"s, &buf) == nullptr) return lastErrorPtr();
    this->SetSpringConstant(GSUtil::Double(buf.c_str()));
    if (GetAttribute("Area"s, &buf) == nullptr) return lastErrorPtr();
    this->SetArea(GSUtil::Double(buf.c_str()));
    if (GetAttribute("Damping"s, &buf) == nullptr) return lastErrorPtr();
    this->SetDamping(GSUtil::Double(buf.c_str()));
    if (GetAttribute("BreakingStrain"s, &buf) == nullptr) return lastErrorPtr();
    this->SetBreakingStrain(GSUtil::Double(buf.c_str()));
    return nullptr;
}

 void DampedSpringMuscle::AppendToAttributes()
{
     Muscle::AppendToAttributes();
    std::string buf;
    setAttribute("Type"s, "DampedSpring"s);
    setAttribute("UnloadedLength"s, *GSUtil::ToString(m_UnloadedLength, &buf));
    setAttribute("SpringConstant"s, *GSUtil::ToString(m_SpringConstant, &buf));
    setAttribute("Area"s, *GSUtil::ToString(m_Area, &buf));
    setAttribute("Damping"s, *GSUtil::ToString(m_Damping, &buf));
    setAttribute("BreakingStrain"s, *GSUtil::ToString(m_BreakingStrain, &buf));
}

