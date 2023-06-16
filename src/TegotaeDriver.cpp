 /*
 *  TegotaeDriver.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#include "TegotaeDriver.h"

#include "Body.h"
#include "Geom.h"
#include "Contact.h"
#include "Marker.h"
#include "GSUtil.h"
#include "Drivable.h"
#include "Muscle.h"
#include "Controller.h"

#include "pystring.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std::string_literals;

TegotaeDriver::TegotaeDriver()
{
}

void TegotaeDriver::Initialise(double omega, double sigma, double A, double Aprime, double B, double phi,
                               Marker *tegotaeCentre, Marker *tegotaeRim, Marker *errorOutput, Marker *forceDirection,
                               const std::vector<Geom *> &contactGeomList)
{
    m_omega   =   omega;        // rad s-1     intrinsic angular velocity
    m_sigma   =   sigma;        // rad s-1 N-1 magnitude of sensory feedback
    m_A       =   A;            // m           positive y-direction amplitude of leg motion in a swing phase (up)
    m_Aprime  =   Aprime;       // m           negative y-direction amplitude of leg motion in a stance phase (down)
    m_B       =   B;            // m           x-direction amplitude of leg motion (forward and backwards)
    m_phi     =   phi;          // rad         initial phase on LF leg's oscillator

    m_N       =   0;            // N           ground reaction force

    m_tegotaeCentre = tegotaeCentre;              // this marker defines the centre position and local coordinate system for the controller
    m_errorOutput = errorOutput;                  // this marker defines the position but not direction used to output the error system
    m_forceDirection = forceDirection;            // this marker defines the direction for the contact force feedback
    m_contactGeomList = contactGeomList;          // these are the geoms that are used for the contact force feedback

    // and set the derived values
    m_phi_dot = m_omega;
    m_X = m_B * std::cos(m_phi);                   // X (0<= m_phi < 2pi)
    if (m_phi < M_PI) m_Y = m_A * std::sin(m_phi); // Y (0<= m_phi < pi)
    else m_Y = m_Aprime * std::sin(m_phi);         // Y (pi<= m_phi < 2pi)

    pgd::Quaternion rimLocalQ = m_tegotaeCentre->GetQuaternion();
    pgd::Vector3 rimWorldP = m_tegotaeCentre->GetWorldPosition(pgd::Vector3(m_X, m_Y, 0));
    m_tegotaeRim = tegotaeRim;
    m_tegotaeRim->SetQuaternion(rimLocalQ.n, rimLocalQ.x, rimLocalQ.y, rimLocalQ.z);
    m_tegotaeRim->SetWorldPosition(rimWorldP.x ,rimWorldP.y, rimWorldP.z);

}

void TegotaeDriver::SendData()
{
    for (auto &&it : *targetList())
    {
        it.second->ReceiveData(Clamp(m_localErrorVector.x), simulation()->GetStepCount());
    }
    for (auto &&it : m_targetList1)
    {
        it.second->ReceiveData(Clamp(m_localErrorVector.y), simulation()->GetStepCount());
    }
    for (auto &&it : m_targetList2)
    {
        it.second->ReceiveData(Clamp(m_localErrorVector.z), simulation()->GetStepCount());
    }
}

void TegotaeDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());

    if (m_omegaDriver) m_omega = m_omegaDriver->value();
    if (m_sigmaDriver) m_sigma = m_sigmaDriver->value();
    if (m_ADriver) m_A = m_ADriver->value();
    if (m_AprimeDriver) m_Aprime = m_AprimeDriver->value();
    if (m_BDriver) m_B = m_BDriver->value();

    // main control algorithm
    if (m_mirror) m_phi_dot = m_omega + m_sigma * m_N * std::cos(m_phi);    // this version follows the normal gaitsym convention of forward being +X
    else m_phi_dot = m_omega - m_sigma * m_N * std::cos(m_phi);             // this is the version in the paper where -X is forward
    // lets try not letting m_phi_dot be negative
    if (m_allow_negative_phi_dot == false && m_phi_dot < 0) m_phi_dot = 0;

    // leg control
    // +ve X is relative distance forward
    // +ve Y is relative distance away from body
    // m_A is the extra distance during stance phase
    // m_Aprime is distance moved up during swing phase

    m_X = m_B * std::cos(m_phi);                   // X (0<= m_phi < 2pi)
    if (m_phi < M_PI) m_Y = m_A * std::sin(m_phi); // Y (0<= m_phi < pi)
    else m_Y = m_Aprime * std::sin(m_phi);         // Y (pi<= m_phi < 2pi)

    // get the world position of the Tegotae target
    pgd::Quaternion rimLocalQ = m_tegotaeCentre->GetQuaternion();
    pgd::Vector3 rimWorldP = m_tegotaeCentre->GetWorldPosition(pgd::Vector3(m_X, m_Y, 0));
    m_tegotaeRim->SetQuaternion(rimLocalQ.n, rimLocalQ.x, rimLocalQ.y, rimLocalQ.z);
    m_tegotaeRim->SetWorldPosition(rimWorldP.x ,rimWorldP.y, rimWorldP.z);
    pgd::Vector3 targetDesiredPosition = m_errorOutput->GetWorldPosition();
    m_worldErrorVector = rimWorldP - targetDesiredPosition;
    m_localErrorVector = m_tegotaeCentre->GetVector(m_worldErrorVector); // this should mean that the position depends on m_errorOutput but direction depends on m_tegotaeCentre

    // update m_phi depending on m_phi_dot values
    double deltaT = simulation()->GetTimeIncrement();
    m_phi = std::fmod(m_phi + m_phi_dot * deltaT, 2 * M_PI);
}

void TegotaeDriver::UpdateReactionForce()
{
    // N is the ground reaction force (GRF) acting on the leg
    m_N = 0;
    pgd::Vector3 worldXAxis;
    pgd::Vector3 worldReactionForce;
    for (auto geomIt : m_contactGeomList)
    {
        std::vector<Contact *> *contactList = geomIt->GetContactList();
        dJointFeedback *jointFeedback;
        for (unsigned int i = 0; i < contactList->size(); i++)
        {
            jointFeedback = contactList->at(i)->GetJointFeedback();
            // add the force that matches the X direction of the marker
            worldXAxis = m_forceDirection->GetWorldAxis(Marker::Axis::X);
            worldReactionForce.Set(jointFeedback->f1);
            m_N += pgd::Dot(worldXAxis, worldReactionForce);
        }
    }
    if (m_N < 0) m_N = 0;
}

std::string TegotaeDriver::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tomega\tsigma\tA\tAprime\tB"
           << "\tX\tY\tN\tphi\tphi_dot"
           << "\terrorX\terrorY\terrorZ"
           << "\n";
    }

    ss << simulation()->GetTime() << "\t" << m_omega << "\t" << m_sigma << "\t" << m_A << "\t" << m_Aprime << "\t" << m_B << "\t" <<
          m_X << "\t" << m_Y << "\t" << m_N << "\t" << m_phi << "\t" << m_phi_dot << "\t" <<
          m_localErrorVector.x << "\t" << m_localErrorVector.y << "\t" << m_localErrorVector.z
       << "\n";
    return ss.str();
}

double TegotaeDriver::omega() const
{
    return m_omega;
}

double TegotaeDriver::sigma() const
{
    return m_sigma;
}

double TegotaeDriver::A() const
{
    return m_A;
}

double TegotaeDriver::Aprime() const
{
    return m_Aprime;
}

double TegotaeDriver::B() const
{
    return m_B;
}

double TegotaeDriver::phi() const
{
    return m_phi;
}

double TegotaeDriver::X() const
{
    return m_X;
}

double TegotaeDriver::Y() const
{
    return m_Y;
}

double TegotaeDriver::N() const
{
    return m_N;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *TegotaeDriver::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    double omega, sigma, A, Aprime, B, phi;
    if (findAttribute("Omega"s, &buf) == nullptr) return lastErrorPtr();
    omega = GSUtil::Double(buf);
    if (findAttribute("Sigma"s, &buf) == nullptr) return lastErrorPtr();
    sigma = GSUtil::Double(buf);
    if (findAttribute("A"s, &buf) == nullptr) return lastErrorPtr();
    A = GSUtil::Double(buf);
    if (findAttribute("Aprime"s, &buf) == nullptr) return lastErrorPtr();
    Aprime = GSUtil::Double(buf);
    if (findAttribute("B"s, &buf) == nullptr) return lastErrorPtr();
    B = GSUtil::Double(buf);
    if (findAttribute("Phi"s, &buf) == nullptr) return lastErrorPtr();
    phi = GSUtil::Double(buf);

    if (findAttribute("Mirror"s, &buf)) m_mirror = GSUtil::Double(buf);
    if (findAttribute("AllowNegativePhiDot"s, &buf)) m_allow_negative_phi_dot = GSUtil::Double(buf);

    if (findAttribute("CentreMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *tegotaeCentre = simulation()->GetMarker(buf);
    if (!tegotaeCentre)
    {
        setLastError("TegotaeDriver ID=\""s + name() + "\" CentreMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    if (findAttribute("RimMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *tegotaeRim = simulation()->GetMarker(buf);
    if (!tegotaeRim)
    {
        setLastError("TegotaeDriver ID=\""s + name() + "\" RimMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    if (tegotaeCentre->GetBody() != tegotaeRim->GetBody())
    {
        setLastError("TegotaeDriver ID=\""s + name() + "\" RimMarkerID marker and CentreMarkerID must have the same BODY\"");
        return lastErrorPtr();
    }
    if (findAttribute("ErrorOutputMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *errorOutput = simulation()->GetMarker(buf);
    if (!errorOutput)
    {
        setLastError("TegotaeDriver ID=\""s + name() + "\" ErrorOutputMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    if (findAttribute("ForceDirectionMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    Marker *forceDirection = simulation()->GetMarker(buf);
    if (!forceDirection)
    {
        setLastError("TegotaeDriver ID=\""s + name() + "\" ForceDirectionMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }

    if (findAttribute("ContactGeomIDList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> contactGeomNames;
    pystring::split(buf, contactGeomNames);
    if (contactGeomNames.size() == 0)
    {
        setLastError("TegotaeDriver ID=\""s + name() +"\" No targets found in ContactGeomIDList"s);
        return lastErrorPtr();
    }
    std::vector <Geom *> contactGeomList;
    for (auto &&it : contactGeomNames)
    {
        Geom *contactGeom = simulation()->GetGeom(it);
        if (!contactGeom)
        {
            setLastError("TegotaeDriver ID=\""s + name() + "\" ContactGeomIDList marker not found \""s + it + "\"");
            return lastErrorPtr();
        }
        contactGeomList.push_back(contactGeom);
    }

    if (findAttribute("TargetIDList1"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<NamedObject *> upstreamObjects;
    std::vector<std::string> targetNames;
    pystring::split(buf, targetNames);
    m_targetList1.clear();
    for (size_t i = 0; i < targetNames.size(); i++)
    {
        auto muscleIter = simulation()->GetMuscleList()->find(targetNames[i]);
        if (muscleIter != simulation()->GetMuscleList()->end())
        {
            m_targetList1[muscleIter->first] = muscleIter->second.get();
            upstreamObjects.push_back(muscleIter->second.get());
        }
        else
        {
            auto controllerIter = simulation()->GetControllerList()->find(targetNames[i]);
            if (controllerIter != simulation()->GetControllerList()->end())
            {
                m_targetList1[controllerIter->first] = controllerIter->second.get();
                upstreamObjects.push_back(controllerIter->second.get());
            }
            else
            {
                setLastError("Driver ID=\""s + name() +"\" TargetIDList1=\""s + buf + "\" not found"s);
                return lastErrorPtr();
            }
        }
    }

    if (findAttribute("TargetIDList2"s, &buf) == nullptr) return lastErrorPtr();
    pystring::split(buf, targetNames);
    m_targetList2.clear();
    for (size_t i = 0; i < targetNames.size(); i++)
    {
        auto muscleIter = simulation()->GetMuscleList()->find(targetNames[i]);
        if (muscleIter != simulation()->GetMuscleList()->end())
        {
            m_targetList2[muscleIter->first] = muscleIter->second.get();
            upstreamObjects.push_back(muscleIter->second.get());
        }
        else
        {
            auto controllerIter = simulation()->GetControllerList()->find(targetNames[i]);
            if (controllerIter != simulation()->GetControllerList()->end())
            {
                m_targetList2[controllerIter->first] = controllerIter->second.get();
                upstreamObjects.push_back(controllerIter->second.get());
            }
            else
            {
                setLastError("Driver ID=\""s + name() +"\" TargetIDList2=\""s + buf + "\" not found"s);
                return lastErrorPtr();
            }
        }
    }

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
    if (findAttribute("ADriverID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" ADriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_ADriver = driver;
    }
    if (findAttribute("AprimeDriverID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" AprimeDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_AprimeDriver = driver;
    }
    if (findAttribute("BDriverID"s, &buf))
    {
        auto driver = simulation()->GetDriver(buf);
        if (!driver) { setLastError("Driver ID=\""s + name() +"\" BDriverID=\""s + buf + "\" not found"s); return lastErrorPtr(); }
        m_BDriver = driver;
    }

    Initialise(omega, sigma, A, Aprime, B, phi, tegotaeCentre, tegotaeRim, errorOutput, forceDirection, contactGeomList);

    upstreamObjects.push_back(m_tegotaeCentre);
    upstreamObjects.push_back(m_tegotaeRim);
    upstreamObjects.push_back(m_errorOutput);
    upstreamObjects.push_back(m_forceDirection);
    for (auto &&it : m_contactGeomList) upstreamObjects.push_back(it);
    if (m_omegaDriver) upstreamObjects.push_back(m_omegaDriver);
    if (m_sigmaDriver) upstreamObjects.push_back(m_sigmaDriver);
    if (m_ADriver) upstreamObjects.push_back(m_ADriver);
    if (m_AprimeDriver) upstreamObjects.push_back(m_AprimeDriver);
    if (m_BDriver) upstreamObjects.push_back(m_BDriver);
    setUpstreamObjects(std::move(upstreamObjects) );
    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void TegotaeDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "Tegotae"s);
    setAttribute("Omega"s, *GSUtil::ToString(m_omega, &buf));
    setAttribute("Sigma"s, *GSUtil::ToString(m_sigma, &buf));
    setAttribute("A"s, *GSUtil::ToString(m_A, &buf));
    setAttribute("Aprime"s, *GSUtil::ToString(m_Aprime, &buf));
    setAttribute("B"s, *GSUtil::ToString(m_B, &buf));
    setAttribute("Phi"s, *GSUtil::ToString(m_phi, &buf));
    setAttribute("Mirror"s, *GSUtil::ToString(m_mirror, &buf));
    setAttribute("AllowNegativePhiDot"s, *GSUtil::ToString(m_allow_negative_phi_dot, &buf));
    setAttribute("CentreMarkerID"s, m_tegotaeCentre->name());
    setAttribute("RimMarkerID"s, m_tegotaeRim->name());
    setAttribute("ErrorOutputMarkerID"s, m_errorOutput->name());
    setAttribute("ForceDirectionMarkerID"s, m_forceDirection->name());
    std::vector<std::string> stringList;
    stringList.reserve(m_contactGeomList.size());
    for (auto &&it : m_contactGeomList) stringList.push_back(it->name());
    setAttribute("ContactGeomIDList"s, pystring::join(" "s, stringList));
    stringList.clear();
    stringList.reserve(m_targetList1.size());
    for (auto &&it : m_targetList1) stringList.push_back(it.first);
    setAttribute("TargetIDList1"s, pystring::join(" "s, stringList));
    stringList.clear();
    stringList.reserve(m_targetList2.size());
    for (auto &&it : m_targetList2) stringList.push_back(it.first);
    setAttribute("TargetIDList2"s, pystring::join(" "s, stringList));
    if (m_omegaDriver) setAttribute("OmegaDriverID"s, m_omegaDriver->name());
    if (m_sigmaDriver) setAttribute("SigmaDriverID"s, m_sigmaDriver->name());
    if (m_ADriver) setAttribute("ADriverID"s, m_ADriver->name());
    if (m_AprimeDriver) setAttribute("AprimeDriverID"s, m_AprimeDriver->name());
    if (m_BDriver) setAttribute("BDriverID"s, m_BDriver->name());
}

pgd::Vector3 TegotaeDriver::worldErrorVector() const
{
    return m_worldErrorVector;
}

pgd::Vector3 TegotaeDriver::localErrorVector() const
{
    return m_localErrorVector;
}


