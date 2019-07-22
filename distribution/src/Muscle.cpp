/*
 *  Muscle.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */


#include "Muscle.h"



#include <string>

using namespace std::string_literals;

Muscle::Muscle()
{
}

Muscle::~Muscle()
{
}

std::string *Muscle::CreateFromAttributes()
{
    if (NamedObject::CreateFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (GetAttribute("StrapID"s, &buf) == nullptr) return lastErrorPtr();
    auto *strapList = simulation()->GetStrapList();
    auto it = strapList->find(buf);
    if (it == strapList->end())
    {
        setLastError("MUSCLE ID=\""s + GetName() +"\" StrapID=\""s + buf + "\" not found"s);
        return lastErrorPtr();
    }
    this->SetStrap(it->second);
    return nullptr;
}

void Muscle::SaveToAttributes()
{
    this->setTag("MUSCLE"s);
    this->AppendToAttributes();
}

void Muscle::AppendToAttributes()
{
    NamedObject::AppendToAttributes();
    std::string buf;
    setAttribute("StrapID"s, this->GetStrap()->GetName());
    return;
}

double Muscle::GetLength() { return m_Strap->GetLength(); }

double Muscle::GetVelocity() { return m_Strap->GetVelocity(); }

double Muscle::GetTension() { return m_Strap->GetTension(); }

double Muscle::GetPower() { return -m_Strap->GetTension() * m_Strap->GetVelocity(); }

void Muscle::CalculateStrap(double simulationTime) { m_Strap->Calculate(simulationTime); }

std::vector<PointForce *> *Muscle::GetPointForceList() { return m_Strap->GetPointForceList(); }

Strap *Muscle::GetStrap() { return m_Strap; }

void Muscle::SetStrap(Strap *strap) { m_Strap = strap; }

int Muscle::SanityCheck(Muscle *otherMuscle, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight)
{
    return m_Strap->SanityCheck(otherMuscle->m_Strap, axis, sanityCheckLeft, sanityCheckRight);
}
void Muscle::LateInitialisation()
{
    CalculateStrap(-1);
    m_Strap->updateDependentMarkers();
}



