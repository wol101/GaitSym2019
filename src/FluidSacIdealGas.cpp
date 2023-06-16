/*
 *  FluidSacIdealGas.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 02/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "FluidSacIdealGas.h"

#include "Marker.h"
#include "Strap.h"
#include "GSUtil.h"

#include "ode/ode.h"

#include "pystring.h"

using namespace std::string_literals;

FluidSacIdealGas::FluidSacIdealGas()
{
}

void FluidSacIdealGas::calculatePressure()
{
//    The ideal gas law (generalised gas equation) is:
//        P V = n R T = N kB T
//
//    where:
//        P is the pressure of the gas,
//        V is the volume of the gas,
//        n is the amount of substance of gas (also known as number of moles),
//        N is the number of gas molecules (or the Avogadro constant times the amount of substance),
//        R is the ideal, or universal, gas constant, equal to the product of the Boltzmann constant and the Avogadro constant,
//        kB is the Boltzmann constant
//        T is the absolute temperature of the gas.
//
//    In SI units, P is measured in pascals, V is measured in cubic metres, n is measured in moles, and T in kelvins.
//    R has the value 8.314 J/(K·mol) approx 2 cal/(K·mol), or 0.08206 L·atm/(mol·K).
//
//    The external pressure used here is commonly the normal temperature and pressure (abbreviated as NTP)
//    which has a value of 101.325 kPa (T=20 degrees C (293.15 K, 68 degrees F).
//    Standard conditions for temperature and pressure (STP) is T=273.15 K (0  degrees C, 32 degrees F)
//    and an absolute pressure of exactly 10e5 Pa

    setPressure((m_amountOfSubstance * m_R * m_temperature / sacVolume()) - m_externalPressure);
}

void FluidSacIdealGas::LateInitialisation()
{
    this->calculateVolume();
    if (m_amountOfSubstance < 0)
    {
        setPressure(0);
        m_amountOfSubstance = sacVolume() * (pressure() + m_externalPressure) / (m_R * m_temperature);
    }
    FluidSac::LateInitialisation();
}

std::string *FluidSacIdealGas::createFromAttributes()
{
    if (FluidSac::createFromAttributes()) return lastErrorPtr();
    std::string buf;

    if (findAttribute("AmountOfSubstance"s, &buf) == nullptr) return lastErrorPtr();
    this->setAmountOfSubstance(GSUtil::Double(buf));
    if (findAttribute("ExternalPressure"s, &buf) == nullptr) return lastErrorPtr();
    this->setExternalPressure(GSUtil::Double(buf));
    if (findAttribute("Temperature"s, &buf) == nullptr) return lastErrorPtr();
    this->setTemperature(GSUtil::Double(buf));

    return nullptr;
}

void FluidSacIdealGas::appendToAttributes()
{
    FluidSac::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "IdealGas"s);
    setAttribute("AmountOfSubstance"s, GSUtil::ToString(m_amountOfSubstance));
    setAttribute("ExternalPressure"s, GSUtil::ToString(m_externalPressure));
    setAttribute("Temperature"s, GSUtil::ToString(m_temperature));
}

double FluidSacIdealGas::temperature() const
{
    return m_temperature;
}

void FluidSacIdealGas::setTemperature(double temperature)
{
    m_temperature = temperature;
}

double FluidSacIdealGas::amountOfSubstance() const
{
    return m_amountOfSubstance;
}

void FluidSacIdealGas::setAmountOfSubstance(double amountOfSubstance)
{
    m_amountOfSubstance = amountOfSubstance;
}

double FluidSacIdealGas::externalPressure() const
{
    return m_externalPressure;
}

void FluidSacIdealGas::setExternalPressure(double externalPressure)
{
    m_externalPressure = externalPressure;
}

double FluidSacIdealGas::R() const
{
    return m_R;
}

void FluidSacIdealGas::setR(double R)
{
    m_R = R;
}
