/*
 *  MAMuscle.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// MAMuscle - implementation of an Minetti & Alexander style
// muscle based on the StrapForce class

// Minetti & Alexander, J. theor Biol (1997) 186, 467-476

// Added extra terms to allow a parallel spring element

#include "ode/ode.h"


#include "Strap.h"
#include "MAMuscle.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "TwoPointStrap.h"
#include "NPointStrap.h"
#include "CylinderWrapStrap.h"
#include "TwoCylinderWrapStrap.h"

using namespace std::string_literals;

// constructor

MAMuscle::MAMuscle(): Muscle()
{
}

// destructor
MAMuscle::~MAMuscle()
{
}

// set the proportion of muscle fibres that are active
// calculates the tension in the strap

void MAMuscle::SetAlpha(double alpha)
{
    double fCE;
    double v, fFull;

    if (alpha < 0) m_Alpha = 0;
    else
    {
        if (alpha > 1.0) m_Alpha = 1.0;
        else m_Alpha = alpha;
    }

    // m_Velocity is negative when muscle shortening
    // we need the sign the other way round
    v = -GetStrap()->GetVelocity();

    // limit v
    if (v > m_VMax) v = m_VMax;
    else if (v < -m_VMax) v = -m_VMax;

    if (v < 0)
    {
        fFull = m_F0 * (1.8 - 0.8 * ((m_VMax + v) / (m_VMax - (7.56 / m_K) * v)));
    }
    else
    {
        fFull = m_F0 * (m_VMax - v) / (m_VMax + (v / m_K));
    }

    // now set the tension as a proportion of fFull
    fCE = m_Alpha * fFull;
    GetStrap()->SetTension(fCE);
}

// calculate the metabolic power of the muscle

double MAMuscle::GetMetabolicPower()
{
    // m_Velocity is negative when muscle shortening
    // we need the sign the other way round
    double relV = -GetStrap()->GetVelocity() / m_VMax;

    // limit relV
    if (relV > 1) relV = 1;
    else if (relV < -1) relV = -1;

    double relVSquared = relV * relV;
    double relVCubed = relVSquared * relV;

    double sigma = (0.054 + 0.506 * relV + 2.46 * relVSquared) /
        (1 - 1.13 * relV + 12.8 * relVSquared - 1.64 * relVCubed);

    return (m_Alpha * m_F0 * m_VMax * sigma);
}

std::string *MAMuscle::CreateFromAttributes()
{
    if (Muscle::CreateFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (GetAttribute("ForcePerUnitArea"s, &buf) == nullptr) return lastErrorPtr();
    m_forcePerUnitArea = GSUtil::Double(buf);
    if (GetAttribute("VMaxFactor"s, &buf) == nullptr) return lastErrorPtr();
    m_vMaxFactor = GSUtil::Double(buf);
    if (GetAttribute("PCA"s, &buf) == nullptr) return lastErrorPtr();
    m_pca = GSUtil::Double(buf);
    this->SetF0(m_pca * m_forcePerUnitArea);
    if (GetAttribute("FibreLength"s, &buf) == nullptr) return lastErrorPtr();
    m_fibreLength = GSUtil::Double(buf);
    this->SetVMax(m_fibreLength * m_vMaxFactor);
    if (GetAttribute("ActivationK"s, &buf) == nullptr) return lastErrorPtr();
    m_K = GSUtil::Double(buf);
    return nullptr;
}

void MAMuscle::AppendToAttributes()
{
    Muscle::AppendToAttributes();
    std::string buf;
    setAttribute("Type"s, "MinettiAlexander"s);
    setAttribute("ForcePerUnitArea"s, *GSUtil::ToString(m_forcePerUnitArea, &buf));
    setAttribute("VMaxFactor"s, *GSUtil::ToString(m_vMaxFactor, &buf));
    setAttribute("PCA"s, *GSUtil::ToString(m_pca, &buf));
    setAttribute("FibreLength"s, *GSUtil::ToString(m_fibreLength, &buf));
    setAttribute("ActivationK"s, *GSUtil::ToString(m_K, &buf));
}

void MAMuscle::Dump()
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
            *dumpStream() << "Time\tVMax\tF0\tK\tAlpha\tFCE\tLCE\tVCE\tPMECH\tPMET\n";
        }
    }


    if (dumpStream())
    {
        *dumpStream() << simulation()->GetTime() << "\t" << m_VMax << "\t" << m_F0 << "\t" << m_K << "\t" << m_Alpha <<
                "\t" << GetStrap()->GetTension() << "\t" << GetStrap()->GetLength() << "\t" << GetStrap()->GetVelocity() <<
                "\t" << GetStrap()->GetVelocity() * GetStrap()->GetTension() << "\t" << GetMetabolicPower() <<
                "\n";
    }
}


