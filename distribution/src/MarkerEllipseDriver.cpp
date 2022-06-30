 /*
 *  MarkerEllipseDriver.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#include "MarkerEllipseDriver.h"

#include "Body.h"
#include "Marker.h"
#include "GSUtil.h"
#include "Drivable.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std::string_literals;

MarkerEllipseDriver::MarkerEllipseDriver()
{
}

void MarkerEllipseDriver::Initialise(double omega, double sigma, double XR, double YR, double phi, Marker *markerEllipseCentre, Marker *markerEllipseRim, DataTarget *phaseControlInput)
{
    m_omega   =   omega;        // rad s-1     intrinsic angular velocity
    m_sigma   =   sigma;        //             gain for the phase correction
    m_XR      =   XR;           // m           x-direction radius
    m_YR      =   YR;           // m           y-direction radius
    m_phi     =   phi;          // rad         initial phase on LF leg's oscillator
    m_markerEllipseCentre = markerEllipseCentre;  // this marker defines the centre position and local coordinate system for the controller

    // and set the derived values
    m_phiDot = m_omega;
    m_X = m_XR * std::cos(m_phi);
    m_Y = m_YR * std::sin(m_phi);

    pgd::Quaternion rimLocalQ = m_markerEllipseCentre->GetQuaternion();
    pgd::Vector3 rimWorldP = m_markerEllipseCentre->GetWorldPosition(pgd::Vector3(m_X, m_Y, 0));
    m_markerEllipseRim = markerEllipseRim;
    m_markerEllipseRim->SetQuaternion(rimLocalQ.n, rimLocalQ.x, rimLocalQ.y, rimLocalQ.z);
    m_markerEllipseRim->SetWorldPosition(rimWorldP.x ,rimWorldP.y, rimWorldP.z);
    m_phaseControlInput = phaseControlInput;
}

void MarkerEllipseDriver::SendData()
{
    for (auto &&it : *targetList())
    {
        it.second->ReceiveData(Clamp(std::sqrt(SQUARE(m_X) + SQUARE(m_Y))), simulation()->GetStepCount());
    }
}

void MarkerEllipseDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());

    if (m_omegaDriver) m_omega = m_omegaDriver->value();
    if (m_sigmaDriver) m_sigma = m_sigmaDriver->value();
    if (m_XRDriver) m_XR = m_XRDriver->value();
    if (m_YRDriver) m_YR = m_YRDriver->value();

    // main control algorithm
    if (!m_phaseControlInput)
    {
        m_phiDot = m_omega;
    }
    else
    {
        // we need to do something to correct for the phase
        m_valueChangeDirection = detectSignChange(m_phaseControlInput->calculateError(simulation()->GetTime()));
        if (m_valueChangeDirection != 0)
        {
            m_halfPeriod = simulation()->GetTime() - m_lastPhaseChangeTime;
            m_lastPhaseChangeTime = simulation()->GetTime();
            m_phiDot = std::clamp(M_PI / (m_halfPeriod * m_periodMultiplier), 0.0, m_maxPhiDot); // this copes with halfPeriod of zero since divide by zero is +/- infinity
            // but we need to tweak m_phiDot to get the phase relationship eventually
            if (m_valueChangeDirection > 0) m_wantedPhi = std::fmod(2 * M_PI + std::fmod(M_PI_2 + m_phaseOffset, 2 * M_PI), 2 * M_PI);
            else m_wantedPhi = std::fmod(2 * M_PI + std::fmod(3 * M_PI_2 + m_phaseOffset, 2 * M_PI), 2 * M_PI);
            m_delPhi = m_wantedPhi - m_phi;
            if (std::fabs(m_delPhi) > M_PI) m_delPhi = 2 * M_PI + m_phi - m_wantedPhi;
            m_phiDot += m_sigma * -m_delPhi; // this is a P controller. A PID controller might be better (certainly a PD controller)
        }
    }

    // update m_phi depending on m_phi_dot values
    m_phi = std::fmod(2 * M_PI + std::fmod(m_phi + m_phiDot * simulation()->GetTimeIncrement(), 2 * M_PI), 2 * M_PI); // do fmod twice to get a value from 0 to 2pi

    m_X = m_XR * std::cos(m_phi);
    m_Y = m_YR * std::sin(m_phi);

    // get the world position of the MarkerEllipse target
    pgd::Quaternion rimLocalQ = m_markerEllipseCentre->GetQuaternion();
    pgd::Vector3 rimWorldP = m_markerEllipseCentre->GetWorldPosition(pgd::Vector3(m_X, m_Y, 0));
    m_markerEllipseRim->SetQuaternion(rimLocalQ.n, rimLocalQ.x, rimLocalQ.y, rimLocalQ.z);
    m_markerEllipseRim->SetWorldPosition(rimWorldP.x ,rimWorldP.y, rimWorldP.z);

}

int MarkerEllipseDriver::detectSignChange(double value)
{
    double lastValue = m_butterworthFilter.Output();
    m_butterworthFilter.AddNewSample(value);
    double delta = m_butterworthFilter.Output() - lastValue;
//    std::cerr << "delta = " << delta << "\n";
    if (m_phaseStateIncreasing && delta > 0)
    {
        m_phaseStateChangeCount = 0;
        return 0;
    }
    if (!m_phaseStateIncreasing && delta < 0)
    {
        m_phaseStateChangeCount = 0;
        return 0;
    }
    if (m_phaseStateIncreasing && delta < 0)
    {
        m_phaseStateChangeCount++;
        if (m_phaseStateChangeCount > m_phaseStateCountThreshold)
        {
            m_phaseStateIncreasing = false;
            m_phaseStateChangeCount = 0;
            return -1;
        }
        return 0;
    }
    if (!m_phaseStateIncreasing && delta > 0)
    {
        m_phaseStateChangeCount++;
        if (m_phaseStateChangeCount > m_phaseStateCountThreshold)
        {
            m_phaseStateIncreasing = true;
            m_phaseStateChangeCount = 0;
            return +1;
        }
        return 0;
    }
    return 0;
}


std::string MarkerEllipseDriver::dumpToString()
{
    std::string s;
    if (firstDump())
    {
        setFirstDump(false);
        s += dumpHelper({"time", "omega"s, "sigma"s, "phaseOffset"s, "XR"s, "YR"s, "X"s, "Y"s, "phi"s, "phi_dot"s, "wantedPhi"s, "delPhi"s, "lastPhaseChangeTime"s, "halfPeriod"s, "valueChangeDirection"s});
    }
    s += dumpHelper({simulation()->GetTime(), m_omega, m_sigma, m_phaseOffset, m_XR, m_YR, m_X, m_Y, m_phi, m_phiDot, m_wantedPhi, m_delPhi, m_lastPhaseChangeTime, m_halfPeriod, double(m_valueChangeDirection)});
    return s;
}

double MarkerEllipseDriver::omega() const
{
    return m_omega;
}

double MarkerEllipseDriver::sigma() const
{
    return m_sigma;
}

double MarkerEllipseDriver::phi() const
{
    return m_phi;
}

double MarkerEllipseDriver::X() const
{
    return m_X;
}

double MarkerEllipseDriver::Y() const
{
    return m_Y;
}

double MarkerEllipseDriver::XR() const
{
    return m_XR;
}

double MarkerEllipseDriver::YR() const
{
    return m_YR;
}

double MarkerEllipseDriver::phi_dot() const
{
    return m_phiDot;
}


// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *MarkerEllipseDriver::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    double omega, sigma, XR, YR, phi;
    if (findAttribute("Omega"s, &buf) == nullptr) return lastErrorPtr();
    omega = GSUtil::Double(buf);
    if (findAttribute("Sigma"s, &buf) == nullptr) return lastErrorPtr();
    sigma = GSUtil::Double(buf);
    if (findAttribute("XR"s, &buf) == nullptr) return lastErrorPtr();
    XR = GSUtil::Double(buf);
    if (findAttribute("YR"s, &buf) == nullptr) return lastErrorPtr();
    YR = GSUtil::Double(buf);
    if (findAttribute("Phi"s, &buf) == nullptr) return lastErrorPtr();
    phi = GSUtil::Double(buf);

    if (findAttribute("CentreMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *markerEllipseCentre = simulation()->GetMarker(buf);
    if (!markerEllipseCentre)
    {
        setLastError("MarkerEllipseDriver ID=\""s + name() + "\" CentreMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    if (findAttribute("RimMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *markerEllipseRim = simulation()->GetMarker(buf);
    if (!markerEllipseRim)
    {
        setLastError("MarkerEllipseDriver ID=\""s + name() + "\" RimMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    if (markerEllipseCentre->GetBody() != markerEllipseRim->GetBody())
    {
        setLastError("MarkerEllipseDriver ID=\""s + name() + "\" RimMarkerID marker and CentreMarkerID must have the same BODY\"");
        return lastErrorPtr();
    }
    if (findAttribute("PhaseControlInputID"s, &buf) == nullptr) return lastErrorPtr();
    DataTarget *phaseControlInput = simulation()->GetDataTarget(buf);
    if (!phaseControlInput)
    {
        setLastError("PhaseControlInputID ID=\""s + name() + "\" PhaseControlInputID data target not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    Initialise(omega, sigma, XR, YR, phi, markerEllipseCentre, markerEllipseRim, phaseControlInput);

    if (findAttribute("LowPassFrequency"s, &buf) == nullptr) return lastErrorPtr();
    m_butterworthFilter.CalculateCoefficients(GSUtil::Double(buf), 1.0 / simulation()->GetTimeIncrement());
    if (findAttribute("PhaseOffset"s, &buf) == nullptr) return lastErrorPtr();
    m_phaseOffset = GSUtil::Double(buf);
    if (findAttribute("MaxPhiDot"s, &buf) == nullptr) return lastErrorPtr();
    m_maxPhiDot = GSUtil::Double(buf);
    if (findAttribute("PeriodMultiplier"s, &buf) == nullptr) return lastErrorPtr();
    m_periodMultiplier = GSUtil::Double(buf);

    if (findAttribute("OmegaDriverID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" OmegaDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_omegaDriver = driver;
    }
    if (findAttribute("SigmaDriverID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" SigmaDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_sigmaDriver = driver;
    }
    if (findAttribute("XRDriverID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" ADriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_XRDriver = driver;
    }
    if (findAttribute("YRDriverID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" AprimeDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_YRDriver = driver;
    }


    std::vector<NamedObject *> upstreamObjects;
    upstreamObjects.push_back(m_markerEllipseCentre);
    upstreamObjects.push_back(m_markerEllipseRim);
    upstreamObjects.push_back(m_phaseControlInput);
    if (m_omegaDriver) upstreamObjects.push_back(m_omegaDriver);
    if (m_sigmaDriver) upstreamObjects.push_back(m_sigmaDriver);
    if (m_XRDriver) upstreamObjects.push_back(m_XRDriver);
    if (m_YRDriver) upstreamObjects.push_back(m_YRDriver);
    setUpstreamObjects(std::move(upstreamObjects));
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void MarkerEllipseDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "MarkerEllipse"s);
    setAttribute("Omega"s, *GSUtil::ToString(m_omega, &buf));
    setAttribute("Sigma"s, *GSUtil::ToString(m_sigma, &buf));
    setAttribute("XR"s, *GSUtil::ToString(m_XR, &buf));
    setAttribute("YR"s, *GSUtil::ToString(m_YR, &buf));
    setAttribute("Phi"s, *GSUtil::ToString(m_phi, &buf));
    setAttribute("CentreMarkerID"s, m_markerEllipseCentre->name());
    setAttribute("RimMarkerID"s, m_markerEllipseRim->name());
    setAttribute("PhaseControlInputID"s, m_phaseControlInput->name());
    setAttribute("LowPassFrequency"s, *GSUtil::ToString(m_butterworthFilter.cutoffFrequency(), &buf));
    setAttribute("PhaseOffset"s, *GSUtil::ToString(m_phaseOffset, &buf));
    setAttribute("MaxPhiDot"s, *GSUtil::ToString(m_maxPhiDot, &buf));
    setAttribute("PeriodMultiplier"s, *GSUtil::ToString(m_periodMultiplier, &buf));
    if (m_omegaDriver) setAttribute("OmegaDriverID"s, m_omegaDriver->name());
    if (m_sigmaDriver) setAttribute("SigmaDriverID"s, m_sigmaDriver->name());
    if (m_XRDriver) setAttribute("XRDriverID"s, m_XRDriver->name());
    if (m_YRDriver) setAttribute("YRDriverID"s, m_YRDriver->name());
}
