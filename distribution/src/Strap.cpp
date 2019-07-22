/*
 *  Strap.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "Strap.h"
#include "Body.h"
#include "Simulation.h"
#include "GSUtil.h"



#include <string>

using namespace std::string_literals;

Strap::Strap()
{
}

Strap::~Strap()
{
    std::vector<PointForce *>::const_iterator iter1;
    for (iter1 = PointForceList()->begin(); iter1 != PointForceList()->end(); iter1++)
        delete *iter1;

}

std::string *Strap::CreateFromAttributes()
{
    m_PointForceList.clear();
    m_dependentMarkers.clear();;
    if (NamedObject::CreateFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (GetAttribute("Length"s, &buf)) { m_Length = GSUtil::Double(buf); m_saveLengthFlag = true; }
    return nullptr;
}

void Strap::SaveToAttributes()
{
    this->setTag("STRAP"s);
    this->AppendToAttributes();
}

void Strap::AppendToAttributes()
{
    NamedObject::AppendToAttributes();
    std::string buf;
    if (m_saveLengthFlag) setAttribute("Length"s, *GSUtil::ToString(m_Length, &buf));
    return;
}

std::vector<PointForce *> *Strap::PointForceList()
{
    return &m_PointForceList;
}

double Strap::Length() const
{
    return m_Length;
}

void Strap::setLength(double length, double simulationTime)
{
    // various special cases
    if (simulationTime < 0)
    {
        if (m_saveLengthFlag)
        {
            m_Velocity = (length - m_Length) / simulation()->GetGlobal().StepSize();
            m_Length = length;
            m_LastTime = 0;
        }
        else
        {
            m_Velocity = 0;
            m_Length = length;
            m_LastTime = 0;
        }
        return;
    }
    if (simulationTime > m_LastTime) m_Velocity = (length - m_Length) / simulation()->GetGlobal().StepSize();
    m_LastTime = simulationTime;
    m_Length = length;
    m_saveLengthFlag = true;
}

double Strap::Velocity() const
{
    return m_Velocity;
}

double Strap::Tension() const
{
    return m_Tension;
}

void Strap::setTension(double Tension)
{
    m_Tension = Tension;
}

std::set<Marker *> *Strap::dependentMarkers()
{
    return &m_dependentMarkers;
}

void Strap::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == nullptr)
        {
            if (GetName().size() == 0) std::cerr << "Strap::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tBody\tXP\tYP\tZP\tFX\tFY\tFZ\n";
        }
    }


    if (dumpStream())
    {
        std::vector<PointForce *>::const_iterator iter1;
        for (iter1 = PointForceList()->begin(); iter1 != PointForceList()->end(); iter1++)
        {
            *dumpStream() << simulation()->GetTime() << "\t" << ((*iter1)->body->GetName()) << "\t" <<
                             (*iter1)->point[0] << "\t" << (*iter1)->point[1] << "\t" << (*iter1)->point[2] << "\t" <<
                             (*iter1)->vector[0] * m_Tension << "\t" << (*iter1)->vector[1] * m_Tension << "\t" << (*iter1)->vector[2] * m_Tension << "\n";
        }
    }
}



