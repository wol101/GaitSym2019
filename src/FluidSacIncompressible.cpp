/*
 *  FluidSacIncompressible.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 02/03/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "FluidSacIncompressible.h"

#include "Marker.h"
#include "Strap.h"
#include "GSUtil.h"

#include "ode/ode.h"

#include "pystring.h"

using namespace std::string_literals;

FluidSacIncompressible::FluidSacIncompressible()
{
}

void FluidSacIncompressible::calculatePressure()
{
//    The Bulk Modulus Elasticity can be calculated as
//        K = - dp / (dV / V0)
//           = - (p1 - p0) / ((V1 - V0) / V0)                       (1)
//    where
//        K = Bulk Modulus of Elasticity (Pa, N/m2)
//        dp = differential change in pressure on the object (Pa, N/m2)
//        dV = differential change in volume of the object (m3)
//        V0 = initial volume of the object  (m3)
//        p0 = initial pressure (Pa, N/m2)
//        p1 = final pressure (Pa, N/m2)
//        V1 = final volume (m3)
//    rearranging
//        p1 = p0 - K(v1 - v0) / v0
    double volumeTerm = m_bulkModulus * (sacVolume() - m_fluidVolume) / m_fluidVolume;
    double dotVolumeTerm = dotSacVolume() * m_bulkModulusDamping / m_fluidVolume;
    double pressure = m_startingPressure - volumeTerm + dotVolumeTerm; // note that the signs are set because reducing volume increases pressure
    setPressure(pressure);
}

std::string *FluidSacIncompressible::createFromAttributes()
{
    if (FluidSac::createFromAttributes()) return lastErrorPtr();
    std::string buf;

    if (findAttribute("FluidVolume"s, &buf) == nullptr) return lastErrorPtr();
    this->setFluidVolume(GSUtil::Double(buf));
    if (findAttribute("BulkModulus"s, &buf) == nullptr) return lastErrorPtr();
    this->setBulkModulus(GSUtil::Double(buf));
    if (findAttribute("BulkModulusDamping"s, &buf) == nullptr) return lastErrorPtr();
    this->setBulkModulusDamping(GSUtil::Double(buf));
    if (findAttribute("StartingPressure"s, &buf) == nullptr) return lastErrorPtr();
    this->setStartingPressure(GSUtil::Double(buf));

    return nullptr;
}

void FluidSacIncompressible::appendToAttributes()
{
    FluidSac::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Incompressible"s);
    setAttribute("FluidVolume"s, GSUtil::ToString(m_fluidVolume));
    setAttribute("BulkModulus"s, GSUtil::ToString(m_bulkModulus));
    setAttribute("BulkModulusDamping"s, GSUtil::ToString(m_bulkModulusDamping));
    setAttribute("StartingPressure"s, GSUtil::ToString(m_startingPressure));
}

double FluidSacIncompressible::bulkModulus() const
{
    return m_bulkModulus;
}

void FluidSacIncompressible::setBulkModulus(double bulkModulus)
{
    m_bulkModulus = bulkModulus;
}

double FluidSacIncompressible::fluidVolume() const
{
    return m_fluidVolume;
}

void FluidSacIncompressible::setFluidVolume(double fluidVolume)
{
    m_fluidVolume = fluidVolume;
}

double FluidSacIncompressible::startingPressure() const
{
    return m_startingPressure;
}

void FluidSacIncompressible::setStartingPressure(double startingPressure)
{
    m_startingPressure = startingPressure;
}

double FluidSacIncompressible::bulkModulusDamping() const
{
    return m_bulkModulusDamping;
}

void FluidSacIncompressible::setBulkModulusDamping(double newBulkModulusDamping)
{
    m_bulkModulusDamping = newBulkModulusDamping;
}

