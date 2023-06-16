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

void MarkerEllipseDriver::Initialise(double omega, double sigma, const pgd::Vector4 &XR, const pgd::Vector4 &YR, double phi, Marker *markerEllipseCentre, Marker *markerEllipseRim, DataTarget *phaseControlInput)
{
    m_omega   =   omega;        // rad s-1     intrinsic angular velocity
    m_sigma   =   sigma;        //             gain for the phase correction
    m_XR      =   XR;           // m           x-direction radius
    m_YR      =   YR;           // m           y-direction radius
    m_phi     =   std::fmod(2 * M_PI + std::fmod(phi, 2 * M_PI), 2 * M_PI); // rad initial phase normalised from 0 to 2 pi
    m_markerEllipseCentre = markerEllipseCentre;  // this marker defines the centre position and local coordinate system for the controller

    // and set the derived values
    m_phiDot = m_omega;
    while (true)
    {
        if (m_phi < M_PI_2)
        {
            m_X = m_XR[0] * std::cos(m_phi);
            m_Y = m_YR[0] * std::sin(m_phi);
            break;
        }
        if (m_phi < M_PI)
        {
            m_X = m_XR[1] * std::cos(m_phi);
            m_Y = m_YR[1] * std::sin(m_phi);
            break;
        }
        if (m_phi < 3 * M_PI_2)
        {
            m_X = m_XR[2] * std::cos(m_phi);
            m_Y = m_YR[2] * std::sin(m_phi);
            break;
        }
        m_X = m_XR[3] * std::cos(m_phi);
        m_Y = m_YR[3] * std::sin(m_phi);
        break;
    }

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
    if (m_XRDriver0) m_XR[0] = m_XRDriver0->value();
    if (m_YRDriver0) m_YR[0] = m_YRDriver0->value();
    if (m_XRDriver1) m_XR[1] = m_XRDriver1->value();
    if (m_YRDriver1) m_YR[1] = m_YRDriver1->value();
    if (m_XRDriver2) m_XR[2] = m_XRDriver2->value();
    if (m_YRDriver2) m_YR[2] = m_YRDriver2->value();
    if (m_XRDriver3) m_XR[3] = m_XRDriver3->value();
    if (m_YRDriver3) m_YR[3] = m_YRDriver3->value();

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

    while (true)
    {
        if (m_phi < M_PI_2)
        {
            m_X = m_XR[0] * std::cos(m_phi);
            m_Y = m_YR[0] * std::sin(m_phi);
            break;
        }
        if (m_phi < M_PI)
        {
            m_X = m_XR[1] * std::cos(m_phi);
            m_Y = m_YR[1] * std::sin(m_phi);
            break;
        }
        if (m_phi < 3 * M_PI_2)
        {
            m_X = m_XR[2] * std::cos(m_phi);
            m_Y = m_YR[2] * std::sin(m_phi);
            break;
        }
        m_X = m_XR[3] * std::cos(m_phi);
        m_Y = m_YR[3] * std::sin(m_phi);
        break;
    }

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

/**
 * @brief MarkerEllipseDriver::dumpToString
 * @return string containing the data for this time point
 *
 * This function returns useful data to the user about values contained in this object during the simulation
 *
 * Column Headings:
 *
 * - time
 *   - the simulation time
 * - omega
 *   - the initial angular velocity
 * - sigma
 *   - the phase matching gain
 * - phaseOffset
 *   - the goal phase offset to the target
 * - XR
 *   - the current X radius
 * - YR
 *   - the current Y radius
 * - X
 *   - the current X value (marker local coordinates)
 * - Y
 *   - the current Y value (marker local coordinates)
 * - phi
 *   - the current angle
 * - phi_dot
 *   - the current angular velocity
 * - wantedPhi
 *   - the phiWanted to get the desired phaseOffset
 * - delPhi
 *   - the current change of phi
 * - lastPhaseChangeTime
 *   - the time when the phase was last checked
 * - halfPeriod
 *   - the half period of the driving signal
 * - valueChangeDirection
 *   - the direction that dribing signal is changing (+1, -1 or 0)
 */

std::string MarkerEllipseDriver::dumpToString()
{
    std::string s;
    if (firstDump())
    {
        setFirstDump(false);
        s += dumpHelper({"time", "omega"s, "sigma"s, "phaseOffset"s, "XR"s, "YR"s, "X"s, "Y"s, "phi"s, "phi_dot"s, "wantedPhi"s, "delPhi"s, "lastPhaseChangeTime"s, "halfPeriod"s, "valueChangeDirection"s});
    }
    double XR, YR;
    while (true)
    {
        if (m_phi < M_PI_2)
        {
            XR = m_XR[0];
            YR = m_YR[0];
            break;
        }
        if (m_phi < M_PI)
        {
            XR = m_XR[1];
            YR = m_YR[1];
            break;
        }
        if (m_phi < 3 * M_PI_2)
        {
            XR = m_XR[2];
            YR = m_YR[2];
            break;
        }
        XR = m_XR[3];
        YR = m_YR[3];
        break;
    }

    s += dumpHelper({simulation()->GetTime(), m_omega, m_sigma, m_phaseOffset, XR, YR, m_X, m_Y, m_phi, m_phiDot, m_wantedPhi, m_delPhi, m_lastPhaseChangeTime, m_halfPeriod, double(m_valueChangeDirection)});
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

pgd::Vector4 MarkerEllipseDriver::XR() const
{
    return m_XR;
}

pgd::Vector4 MarkerEllipseDriver::YR() const
{
    return m_YR;
}

double MarkerEllipseDriver::phi_dot() const
{
    return m_phiDot;
}


/**
 * @brief MarkerEllipseDriver::createFromAttributes
 * @return nullptr on success and a pointer to lastError() on failure
 *
 * This function initialises the data in the object based on the contents
 * of an xml_node node. It uses information from the simulation as required
 * to satisfy dependencies
 *
 * Attributes in addition to standard DRIVER:
 *
 * - Type="MarkerEllipse"
 * - Omega="double"
 *   - The initial angular velocity
 * - Sigma="double"
 *   - The phase matching gain
 * - XR="list of doubles"
 *   - The X radius for each quadrant (repeated if only 1 value given)
 * - YR="list of doubles"
 *   - The Y radius for each quadrant (repeated if only 1 value given)
 * - Phi="double"
 *   - The initial phase angle
 * - CentreMarkerID
 *   - ID of the marker that identifies the rotaion centre and axis (rotates around the Z axis, and the X & Y axes are the ones defined for the driver)
 * - RimMarkerID
 *   - ID of the marker that gets moved in the ellipse
 * - PhaseControlInputID
 *   - ID of the data target used to control the phase of the marker's movement
 * - LowPassFrequency
 *   - Low pass filter applied to the phase control signal
 * - PhaseOffset
 *   - Phase offset from the control signal
 * - MaxPhiDot
 *   - Maximum allowable rotational velocity
 * - PeriodMultiplier
 *   - Attempt to multiply the rotational period of the driver signal by this value
 *
 * Optional Attributes
 *
 * - OmegaDriverID
 *   - ID of driver that can change the value of omega
 * - SigmaDriverID
 *   - ID of driver that can change the value of omega
 * - XRDriver0ID
 *   - ID of driver that can change the value of XR in quadrant 0
 * - YRDriver0ID
 *   - ID of driver that can change the value of YR in quadrant 0
 * - XRDriver1ID
 *   - ID of driver that can change the value of XR in quadrant 1
 * - YRDriver1ID
 *   - ID of driver that can change the value of YR in quadrant 1
 * - XRDriver2ID
 *   - ID of driver that can change the value of XR in quadrant 2
 * - YRDriver2ID
 *   - ID of driver that can change the value of YR in quadrant 2
 * - XRDriver3ID
 *   - ID of driver that can change the value of XR in quadrant 3
 * - YRDriver3ID
 *   - ID of driver that can change the value of YR in quadrant 3
 *
 */

std::string *MarkerEllipseDriver::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    double omega, sigma, phi;
    std::vector<double> XR, YR;
    if (findAttribute("Omega"s, &buf) == nullptr) return lastErrorPtr();
    omega = GSUtil::Double(buf);
    if (findAttribute("Sigma"s, &buf) == nullptr) return lastErrorPtr();
    sigma = GSUtil::Double(buf);
    if (findAttribute("XR"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, &XR);
    if (findAttribute("YR"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, &YR);
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
    pgd::Vector4 XRV, YRV;
    if (XR.size() == 1) XRV.Set(XR[0], XR[0], XR[0], XR[0]);
    else for (size_t i = 0; i < XR.size(); i++) { XRV[i] = XR[i]; }
    if (YR.size() == 1) YRV.Set(YR[0], YR[0], YR[0], YR[0]);
    else for (size_t i = 0; i < YR.size(); i++) { YRV[i] = YR[i]; }
    Initialise(omega, sigma, XRV, YRV, phi, markerEllipseCentre, markerEllipseRim, phaseControlInput);

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
    if (findAttribute("XRDriver0ID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" ADriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_XRDriver0 = driver;
    }
    if (findAttribute("YRDriver0ID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" AprimeDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_YRDriver0 = driver;
    }
    if (findAttribute("XRDriver1ID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" ADriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_XRDriver1 = driver;
    }
    if (findAttribute("YRDriver1ID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" AprimeDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_YRDriver1 = driver;
    }
    if (findAttribute("XRDrive2rID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" ADriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_XRDriver2 = driver;
    }
    if (findAttribute("YRDriver2ID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" AprimeDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_YRDriver2 = driver;
    }
    if (findAttribute("XRDriver3ID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" ADriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_XRDriver3 = driver;
    }
    if (findAttribute("YRDriver3ID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" AprimeDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_YRDriver3 = driver;
    }


    std::vector<NamedObject *> upstreamObjects;
    upstreamObjects.push_back(m_markerEllipseCentre);
    upstreamObjects.push_back(m_markerEllipseRim);
    upstreamObjects.push_back(m_phaseControlInput);
    if (m_omegaDriver) upstreamObjects.push_back(m_omegaDriver);
    if (m_sigmaDriver) upstreamObjects.push_back(m_sigmaDriver);
    if (m_XRDriver0) upstreamObjects.push_back(m_XRDriver0);
    if (m_YRDriver0) upstreamObjects.push_back(m_YRDriver0);
    if (m_XRDriver1) upstreamObjects.push_back(m_XRDriver1);
    if (m_YRDriver1) upstreamObjects.push_back(m_YRDriver1);
    if (m_XRDriver2) upstreamObjects.push_back(m_XRDriver2);
    if (m_YRDriver2) upstreamObjects.push_back(m_YRDriver2);
    if (m_XRDriver3) upstreamObjects.push_back(m_XRDriver3);
    if (m_YRDriver3) upstreamObjects.push_back(m_YRDriver3);
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
    setAttribute("XR"s, *GSUtil::ToString(m_XR.data(), 4, &buf));
    setAttribute("YR"s, *GSUtil::ToString(m_YR.data(), 4, &buf));
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
    if (m_XRDriver0) setAttribute("XRDriver0ID"s, m_XRDriver0->name());
    if (m_YRDriver0) setAttribute("YRDriver0ID"s, m_YRDriver0->name());
    if (m_XRDriver1) setAttribute("XRDriver1ID"s, m_XRDriver1->name());
    if (m_YRDriver1) setAttribute("YRDriver1ID"s, m_YRDriver1->name());
    if (m_XRDriver2) setAttribute("XRDriver2ID"s, m_XRDriver2->name());
    if (m_YRDriver2) setAttribute("YRDriver2ID"s, m_YRDriver2->name());
    if (m_XRDriver3) setAttribute("XRDriver3ID"s, m_XRDriver3->name());
    if (m_YRDriver3) setAttribute("YRDriver3ID"s, m_YRDriver3->name());
}
