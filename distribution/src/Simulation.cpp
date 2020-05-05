/*
 *  Simulation.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// Simulation.cpp - this simulation object is used to encapsulate
// a ODE simulation

#include "Simulation.h"

#include "GSUtil.h"
#include "CyclicDriver.h"
#include "StepDriver.h"
#include "DataTarget.h"
#include "DataTargetScalar.h"
#include "DataTargetQuaternion.h"
#include "DataTargetVector.h"
#include "DataFile.h"
#include "PGDMath.h"
#include "Body.h"
#include "HingeJoint.h"
#include "BallJoint.h"
#include "FloatingHingeJoint.h"
#include "CappedCylinderGeom.h"
#include "SphereGeom.h"
#include "Muscle.h"
#include "MAMuscle.h"
#include "MAMuscleComplete.h"
#include "DampedSpringMuscle.h"
#include "TwoPointStrap.h"
#include "CylinderWrapStrap.h"
#include "TwoCylinderWrapStrap.h"
#include "FluidSacIdealGas.h"
#include "FluidSacIncompressible.h"
#include "PlaneGeom.h"
#include "Contact.h"
#include "ErrorHandler.h"
#include "NPointStrap.h"
#include "FixedJoint.h"
#include "TrimeshGeom.h"
#include "Marker.h"
#include "Reporter.h"
#include "TorqueReporter.h"
#include "UniversalJoint.h"
#include "PIDMuscleLength.h"
#include "Controller.h"
#include "SwingClearanceAbortReporter.h"
#include "AMotorJoint.h"
#include "LMotorJoint.h"
#include "SliderJoint.h"
#include "BoxGeom.h"
#include "StackedBoxCarDriver.h"
#include "PIDTargetMatch.h"
#include "Warehouse.h"
#include "FixedDriver.h"
#include "PIDErrorInController.h"
#include "TegotaeDriver.h"

#ifdef USE_QT
#include "FacetedObject.h"
#include "MainWindow.h"
#include "Preferences.h"
#include <QColor>
#endif

#include "ode/ode.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <list>
#include <ctype.h>
#include <cmath>
#include <algorithm>
#include <locale>
#include <codecvt>

using namespace std::string_literals;

// #define _I(i,j) I[(i)*4+(j)]
// regex _I\(([0-9]+),([0-9]+)\) to I[(\1)*4+(\2)]

Simulation::Simulation()
{
    // initialise the ODE world
    dInitODE();
    m_WorldID = dWorldCreate();
    m_SpaceID = dHashSpaceCreate(nullptr); // FIX ME hash space is a compromise but this should probably be user controlled
    m_ContactGroup = dJointGroupCreate(0);
    dSetMessageHandler(ODEMessageTrap);
    dSetErrorHandler(ODEMessageTrap);
    dSetDebugHandler(ODEMessageTrap);
}

//----------------------------------------------------------------------------
Simulation::~Simulation()
{
    // these need to be cleared before we destroy the ODE world
    m_ContactList.clear();
    m_BodyList.clear();
    m_JointList.clear();
    m_GeomList.clear();
    m_MuscleList.clear();
    m_StrapList.clear();
    m_FluidSacList.clear();
    m_DriverList.clear();
    m_DataTargetList.clear();
    m_MarkerList.clear();
    m_ReporterList.clear();
    m_ControllerList.clear();
    m_WarehouseList.clear();

    // destroy the ODE world
    dSetMessageHandler(nullptr);
    dSetErrorHandler(nullptr);
    dSetDebugHandler(nullptr);
    dJointGroupDestroy(m_ContactGroup);
    dSpaceDestroy(m_SpaceID);
    dWorldDestroy(m_WorldID);
    dCloseODE();

}

//----------------------------------------------------------------------------
std::string *Simulation::LoadModel(const char *buffer, size_t length)
{
    std::string *ptr = m_parseXML.LoadModel(buffer, length, "GAITSYM2019"s);
    if (ptr) return ptr;

    auto elementList = m_parseXML.elementList();
    for (auto it = elementList->begin(); it != elementList->end(); it++)
    {
        lastError().clear();
        if (it->get()->tag == "GLOBAL"s) ParseGlobal(it->get());
        else if (it->get()->tag == "BODY"s) ParseBody(it->get());
        else if (it->get()->tag == "JOINT"s) ParseJoint(it->get());
        else if (it->get()->tag == "GEOM"s) ParseGeom(it->get());
        else if (it->get()->tag == "STRAP"s) ParseStrap(it->get());
        else if (it->get()->tag == "MUSCLE"s) ParseMuscle(it->get());
        else if (it->get()->tag == "DRIVER"s) ParseDriver(it->get());
        else if (it->get()->tag == "DATATARGET"s) ParseDataTarget(it->get());
        else if (it->get()->tag == "MARKER"s) ParseMarker(it->get());
        else if (it->get()->tag == "REPORTER"s) ParseReporter(it->get());
        else if (it->get()->tag == "CONTROLLER"s) ParseController(it->get());
        else if (it->get()->tag == "WAREHOUSE"s) ParseWarehouse(it->get());
        else if (it->get()->tag == "FLUIDSAC"s) ParseFluidSac(it->get());
        if (lastError().size()) return lastErrorPtr();
    }

    // joints are set with the bodies in construction poses
    for (auto &&it :  m_JointList) it.second->LateInitialisation();
    // then the bodies are moved to their starting poses
    for (auto &&it : m_BodyList) it.second->LateInitialisation();
    // and we recalculate the dynamic items with the new muscle positions
    for (auto &&it :  m_MuscleList) it.second->LateInitialisation();
    for (auto &&it : m_FluidSacList) it.second->LateInitialisation();

    auto distanceTravelledBodyID = m_BodyList.find(m_global->DistanceTravelledBodyIDName());
    if (distanceTravelledBodyID != m_BodyList.end()) m_DistanceTravelledBodyID = distanceTravelledBodyID->second.get();
    if (m_global->fitnessType() == Global::DistanceTravelled && m_DistanceTravelledBodyID == nullptr)
    {
        setLastError("Error parsing XML file: DistanceTravelledBodyIDName not found - \""s + m_global->DistanceTravelledBodyIDName() +"\""s);
        return lastErrorPtr();
    }

    // for the time being just set the current warehouse to the first one in the list
    if (m_global->CurrentWarehouseFile().length() == 0 && m_WarehouseList.size() > 0) m_global->setCurrentWarehouseFile(m_WarehouseList.begin()->first);

#ifdef OUTPUTS_AFTER_SIMULATION_STEP
    if (m_OutputModelStateAtTime == 0.0 || m_OutputModelStateAtCycle == 0)
    {
        OutputProgramState();
        m_OutputModelStateAtTime = -1.0;
        m_OutputModelStateAtCycle = -1;
    }
#endif

    return nullptr;
}


//----------------------------------------------------------------------------
void Simulation::UpdateSimulation()
{
    // calculate the warehouse and position matching fitnesses before we move to a new location
    if (m_global->fitnessType() != Global::DistanceTravelled)
    {
        if (m_global->fitnessType() == Global::KinematicMatch || m_global->fitnessType() == Global::KinematicMatchMiniMax)
        {

            double minScore = DBL_MAX;
            double matchScore;
            for (auto &&iter3 : m_DataTargetList)
            {
                int lastIndex = iter3.second->GetLastMatchIndex();
                int index = iter3.second->TargetMatch(m_SimulationTime, m_global->StepSize() * 0.50000000001);
                // on rare occasions because of rounding we may get two matches we can check this using the lastIndex since this is the only place where a match is requested
                if (index != -1 && index != lastIndex) // since step size is much smaller than the interval between targets (probably), this should get called exactly once per target time defintion
                {
                    matchScore = iter3.second->GetMatchValue(index);
                    m_KinematicMatchFitness += matchScore;
                    if (matchScore < minScore)
                        minScore = matchScore;
                }
            }
            if (minScore < DBL_MAX)
                m_KinematicMatchMiniMaxFitness += minScore;
        }
        else if (m_global->fitnessType() == Global::KinematicMatchContinuous || m_global->fitnessType() == Global::KinematicMatchContinuousMiniMax)
        {

            double minScore = DBL_MAX;
            double matchScore;
            for (auto &&iter3 : m_DataTargetList)
            {
                matchScore = iter3.second->GetMatchValue(m_SimulationTime);
                m_KinematicMatchFitness += matchScore;
                if (matchScore < minScore)
                    minScore = matchScore;
            }
            if (minScore < DBL_MAX)
                m_KinematicMatchMiniMaxFitness += minScore;
        }
    }

    // now start the actual simulation

    // check collisions first
    dJointGroupEmpty(m_ContactGroup);
    m_ContactList.clear();
    for (auto &&geomIter : m_GeomList) geomIter.second->ClearContacts();
    dSpaceCollide(m_SpaceID, this, &NearCallback);


    // update the drivers
    for (auto &&it : m_DriverList)
    {
        it.second->Update();
        it.second->SendData();
    }
    // and the controllers (which are drivers too probably)
    for (auto &&it : m_ControllerList)
    {
        auto driver = dynamic_cast<Driver *>(it.second.get());
        if (driver)
        {
            driver->Update();
            driver->SendData();
        }
        if (it.second->lastStepCount() != m_StepCount)
            std::cerr << "Warning: " << it.first << " controller not updated\n"; // currently cannot stack controllers although this is fixable
    }

    // update the muscles
    double tension;
    std::vector<std::unique_ptr<PointForce >> *pointForceList;
    PointForce *pointForce;
    for (auto iter1 = m_MuscleList.begin(); iter1 != m_MuscleList.end(); /* no increment */)
    {
        iter1->second->CalculateStrap();
        iter1->second->SetActivation();

        // check for breaking strain
        DampedSpringMuscle *dampedSpringMuscle = dynamic_cast<DampedSpringMuscle *>(iter1->second.get());
        if (dampedSpringMuscle)
        {
            if (dampedSpringMuscle->ShouldBreak())
            {
                iter1 = m_MuscleList.erase(iter1); // erase returns the next iterator [but m_MuscleList.erase(iter1++) would also work and is compatible with older C++ compilers]
                continue;
            }
        }

        pointForceList = iter1->second->GetPointForceList();
        tension = iter1->second->GetTension();
#ifdef DEBUG_CHECK_FORCES
        pgd::Vector force(0, 0, 0);
#endif
        for (unsigned int i = 0; i < pointForceList->size(); i++)
        {
            pointForce = (*pointForceList)[i].get();
            dBodyAddForceAtPos(pointForce->body->GetBodyID(),
                               pointForce->vector[0] * tension, pointForce->vector[1] * tension, pointForce->vector[2] * tension,
                               pointForce->point[0], pointForce->point[1], pointForce->point[2]);
#ifdef DEBUG_CHECK_FORCES
            force += pgd::Vector(pointForce->vector[0] * tension, pointForce->vector[1] * tension, pointForce->vector[2] * tension);
#endif
        }
#ifdef DEBUG_CHECK_FORCES
        std::cerr.setf(std::ios::floatfield, std::ios::fixed);
        std::cerr << iter1->first << " " << force.x << " " << force.y << " " << force.z << "\n";
        std::cerr.unsetf(std::ios::floatfield);
#endif
        iter1++; // this has to be done outside the for definition because erase returns the next iterator
    }

    // update the joints (needed for motors, end stops and stress calculations)
    for (auto &&jointIter : m_JointList) jointIter.second->Update();

    // update the fluid sacs
    for (auto fsIter = m_FluidSacList.begin(); fsIter != m_FluidSacList.end(); fsIter++)
    {
        fsIter->second->calculateVolume();
        fsIter->second->calculatePressure();
        fsIter->second->calculateLoadsOnMarkers();
        for (size_t i = 0; i < fsIter->second->pointForceList().size(); i++)
        {
            const PointForce *pf = &fsIter->second->pointForceList().at(i);
            dBodyAddForceAtPos(pf->body->GetBodyID(), pf->vector[0], pf->vector[1], pf->vector[2], pf->point[0], pf->point[1], pf->point[2]);
        }
    }


#ifndef OUTPUTS_AFTER_SIMULATION_STEP
    if (m_OutputWarehouseFlag) OutputWarehouse();
    if (m_OutputModelStateAtTime >= 0.0)
    {
        if (m_SimulationTime >= m_OutputModelStateAtTime)
        {
            OutputProgramState();
            m_OutputModelStateAtTime = -1;
        }
    }
    else if (m_OutputModelStateAtCycle >= 0 && m_CycleTime >= 0 && m_SimulationTime >= m_CycleTime * m_OutputModelStateAtCycle)
    {
        OutputProgramState();
        m_OutputModelStateAtCycle = -1;
    }
    else if (m_OutputModelStateAtWarehouseDistance > 0 && m_WarehouseDistance >= m_OutputModelStateAtWarehouseDistance)
    {
        OutputProgramState();
        m_OutputModelStateAtWarehouseDistance = 0;
    }
#endif

    // run the simulation
    switch (m_global->stepType())
    {
    case Global::World:
        dWorldStep(m_WorldID, m_global->StepSize());
        break;

    case Global::Quick:
        dWorldQuickStep(m_WorldID, m_global->StepSize());
        break;
    }

    // update the time counter
    m_SimulationTime += m_global->StepSize();

    // update the step counter
    m_StepCount++;

    // calculate the energies
    for (auto &&iter1 : m_MuscleList)
    {
        m_MechanicalEnergy += iter1.second->GetPower() * m_global->StepSize();
        m_MetabolicEnergy += iter1.second->GetMetabolicPower() * m_global->StepSize();
    }
    m_MetabolicEnergy += m_global->BMR() * m_global->StepSize();

    // update any contact force dependent drivers (because only after the simulation is the force valid
    // update the footprint indicator
    if (m_ContactList.size() > 0)
    {
        for (auto &&it : m_DriverList)
        {
            TegotaeDriver *tegotaeDriver = dynamic_cast<TegotaeDriver *>(it.second.get());
            if (tegotaeDriver) tegotaeDriver->UpdateReactionForce();
        }
    }

    // all reporting is done after a simulation step

    DumpObjects();

#ifdef OUTPUTS_AFTER_SIMULATION_STEP
//    if (m_OutputKinematicsFlag && m_StepCount % gDisplaySkip == 0) OutputKinematics();
    if (m_OutputWarehouseFlag) OutputWarehouse();
    if (m_OutputModelStateAtTime > 0.0)
    {
        if (m_SimulationTime >= m_OutputModelStateAtTime)
        {
            OutputProgramState();
            m_OutputModelStateAtTime = 0.0;
        }
    }
    else if (m_OutputModelStateAtCycle >= 0 && m_CycleTime >= 0 && m_SimulationTime >= m_CycleTime * m_OutputModelStateAtCycle)
    {
        OutputProgramState();
        m_OutputModelStateAtCycle = -1;
    }
    else if (m_OutputModelStateAtWarehouseDistance > 0 && m_WarehouseDistance >= m_OutputModelStateAtWarehouseDistance)
    {
        OutputProgramState();
        m_OutputModelStateAtWarehouseDistance = 0;
    }
#endif
}

//----------------------------------------------------------------------------
bool Simulation::TestForCatastrophy()
{
#if defined(USE_QT)
    std::stringstream ss;
#endif

    // first of all check to see that ODE is happy
    if (IsMessage())
    {
        int num;
        const char *messageText = GetLastMessage(&num);
        if (m_AbortOnODEMessage)
        {
#if defined(USE_QT)
            ss << "t=" << m_SimulationTime << "Failed due to ODE warning " << num << " " << messageText;
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "t=" << m_SimulationTime << "Failed due to ODE warning " << num << " " << messageText << "\n";
            return true;
        }
        else
        {
#if defined(USE_QT)
            ss << "t=" << m_SimulationTime << " ODE warning " << num << " " << messageText;
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "t=" << m_SimulationTime << " ODE warning " << num << " " << messageText << "\n";
        }
    }

    // check for simulation error
    if (m_SimulationError)
    {
#if defined(USE_QT)
        ss << "Failed due to simulation error " << m_SimulationError;
        if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
        std::cerr << "Failed due to simulation error " << m_SimulationError << "\n";
        return true;
    }

    // check for contact abort
    if (m_ContactAbort)
    {
#if defined(USE_QT)
        ss << "Failed due to contact abort";
        if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
        std::cerr << "Failed due to contact abort\n";
        return true;
    }

    // check for data target abort
    if (m_DataTargetAbort)
    {
#if defined(USE_QT)
        ss << "Failed due to DataTarget abort";
        if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
        std::cerr << "Failed due to DataTarget abort\n";
        return true;
    }

    // check that all bodies meet velocity and stop conditions

    Body::LimitTestResult p;
    for (auto &&iter1 : m_BodyList)
    {
        p = iter1.second->TestLimits();
        switch (p)
        {
        case Body::WithinLimits:
            break;

        case Body::XPosError:
        case Body::YPosError:
        case Body::ZPosError:
#if defined(USE_QT)
            ss << "Failed due to position error " << p << " in: " << iter1.second->name();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to position error " << p << " in: " << iter1.second->name() << "\n";
            return true;

        case Body::XVelError:
        case Body::YVelError:
        case Body::ZVelError:
#if defined(USE_QT)
            ss << "Failed due to velocity error " << p << " in: " << iter1.second->name();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to velocity error " << p << " in: " << iter1.second->name() << "\n";
            return true;

        case Body::NumericalError:
#if defined(USE_QT)
            ss << "Failed due to numerical error " << p << " in: " << iter1.second->name();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to numerical error " << p << " in: " << iter1.second->name() << "\n";
            return true;
        }
    }

    HingeJoint *j;
    FixedJoint *f;
    int t;
    for (auto &&iter3 : m_JointList)
    {
        j = dynamic_cast<HingeJoint *>(iter3.second.get());
        if (j)
        {
            t = j->TestLimits();
            if (t < 0)
            {
#if defined(USE_QT)
                ss << __FILE__ << "Failed due to LoStopTorqueLimit error in: " << iter3.second->name();
                if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
                std::cerr << "Failed due to LoStopTorqueLimit error in: " << iter3.second->name() << "\n";
                return true;
            }
            else if (t > 0)
            {
#if defined(USE_QT)
                ss << __FILE__ << "Failed due to HiStopTorqueLimit error in: " << iter3.second->name();
                if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
                std::cerr << "Failed due to HiStopTorqueLimit error in: " << iter3.second->name() << "\n";
                return true;
            }
        }

        f = dynamic_cast<FixedJoint *>(iter3.second.get());
        if (f)
        {
            if (f->CheckStressAbort())
            {
#if defined(USE_QT)
                ss << __FILE__ << "Failed due to stress limit error in: " << iter3.second->name() << " " << f->GetLowPassMinStress() << " " << f->GetLowPassMaxStress();
                if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
                std::cerr << "Failed due to stress limit error in: " << iter3.second->name() << " " << f->GetLowPassMinStress() << " " << f->GetLowPassMaxStress() << "\n";
                return true;
            }
        }
    }

    // and test the reporters for stop conditions
    for (auto &&reporterIter : m_ReporterList)
    {
        if (reporterIter.second->ShouldAbort())
        {
#if defined(USE_QT)
            ss << __FILE__ << "Failed due to Reporter Abort in: " << reporterIter.second->name();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to Reporter Abort in: " << reporterIter.second->name() << "\n";
            return true;
        }
    }


    if (m_OutputModelStateOccured && m_AbortAfterModelStateOutput)
    {
#if defined(USE_QT)
        ss << __FILE__ << "Abort because ModelState successfully written";
        if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
        std::cerr << "Abort because ModelState successfully written\n";
        return true;
    }

    return false;
}


//----------------------------------------------------------------------------
double Simulation::CalculateInstantaneousFitness()
{
    const double *p;
    switch (m_global->fitnessType())
    {
    case Global::DistanceTravelled:
        p = m_DistanceTravelledBodyID->GetPosition();
        if (std::isinf(p[0]) || std::isnan(p[0]))
        {
            m_SimulationError = 1;
            return 0;
        }
        else
        {
            return p[0];
        }

    case Global::KinematicMatch:
    case Global::KinematicMatchContinuous:
        return m_KinematicMatchFitness;

    case Global::KinematicMatchMiniMax:
    case Global::KinematicMatchContinuousMiniMax:
        return m_KinematicMatchMiniMaxFitness;
    }
    return 0;
}

std::string *Simulation::ParseGlobal(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Global> global = std::make_unique<Global>();
    global->setSimulation(this);
    global->createAttributeMap(node->attributes);
    std::string *errorMessage = global->createFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }
    this->SetGlobal(std::move(global));
    return nullptr;
}

std::string *Simulation::ParseBody(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Body> body = std::make_unique<Body>(m_WorldID);
    body->setSimulation(this);
    body->createAttributeMap(node->attributes);
    std::string *errorMessage = body->createFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }
    std::string name = body->name();
    m_BodyList[name] = std::move(body);
    return nullptr;
}

std::string *Simulation::ParseMarker(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Marker> marker = std::make_unique<Marker>(nullptr);
    marker->setSimulation(this);
    marker->createAttributeMap(node->attributes);
    std::string *errorMessage = marker->createFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }
    std::string name = marker->name();
    m_MarkerList[name] = std::move(marker);
    return nullptr;
}

std::string *Simulation::ParseJoint(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Joint> joint;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "Hinge"s)
    {
        joint = std::make_unique<HingeJoint>(m_WorldID);
        joint->setSimulation(this);
        joint->createAttributeMap(node->attributes);
        errorMessage = joint->createFromAttributes();
    }
    else if (buf == "Ball"s)
    {
        joint = std::make_unique<BallJoint>(m_WorldID, BallJoint::NoStops);
        joint->setSimulation(this);
        joint->createAttributeMap(node->attributes);
        errorMessage = joint->createFromAttributes();
    }
    else if (buf == "Fixed"s)
    {
        joint = std::make_unique<FixedJoint>(m_WorldID);
        joint->setSimulation(this);
        joint->createAttributeMap(node->attributes);
        errorMessage = joint->createFromAttributes();
    }
    else if (buf == "FloatingHinge"s)
    {
        joint = std::make_unique<FloatingHingeJoint>(m_WorldID);
        joint->setSimulation(this);
        joint->createAttributeMap(node->attributes);
        errorMessage = joint->createFromAttributes();
    }
    else if (buf == "Universal"s)
    {
        joint = std::make_unique<UniversalJoint>(m_WorldID);
        joint->setSimulation(this);
        joint->createAttributeMap(node->attributes);
        errorMessage = joint->createFromAttributes();
    }
    else if (buf == "AMotor"s)
    {
        joint = std::make_unique<AMotorJoint>(m_WorldID);
        joint->setSimulation(this);
        joint->createAttributeMap(node->attributes);
        errorMessage = joint->createFromAttributes();
    }
    else if (buf == "LMotor"s)
    {
        joint = std::make_unique<LMotorJoint>(m_WorldID);
        joint->setSimulation(this);
        joint->createAttributeMap(node->attributes);
        errorMessage = joint->createFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseJoint Type=\"" + buf + "\" not recognised");
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }
    std::string name = joint->name();
    m_JointList[name] = std::move(joint);
    return nullptr;
}

std::string *Simulation::ParseGeom(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Geom> geom;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "Box"s)
    {
        std::unique_ptr<BoxGeom> boxGeom = std::make_unique<BoxGeom>(m_SpaceID, 1.0, 1.0, 1.0);
        boxGeom->setSimulation(this);
        boxGeom->createAttributeMap(node->attributes);
        errorMessage = boxGeom->createFromAttributes();
        geom = std::move(boxGeom);
    }
    else if (buf == "CappedCylinder"s)
    {
        std::unique_ptr<CappedCylinderGeom> cappedCylinderGeom = std::make_unique<CappedCylinderGeom>(m_SpaceID, 1.0, 1.0);
        cappedCylinderGeom->setSimulation(this);
        cappedCylinderGeom->createAttributeMap(node->attributes);
        errorMessage = cappedCylinderGeom->createFromAttributes();
        geom = std::move(cappedCylinderGeom);
    }
    else if (buf == "Plane"s)
    {
        std::unique_ptr<PlaneGeom> planeGeom = std::make_unique<PlaneGeom>(m_SpaceID, 10.0, 0.0, 1.0, 0.0);
        planeGeom->setSimulation(this);
        planeGeom->createAttributeMap(node->attributes);
        errorMessage = planeGeom->createFromAttributes();
        geom = std::move(planeGeom);
    }
    else if (buf == "Sphere"s)
    {
        std::unique_ptr<SphereGeom> sphereGeom = std::make_unique<SphereGeom>(m_SpaceID, 1.0);
        sphereGeom->setSimulation(this);
        sphereGeom->createAttributeMap(node->attributes);
        errorMessage = sphereGeom->createFromAttributes();
        geom = std::move(sphereGeom);
    }
    else
    {
        setLastError("Simulation::ParseGeom Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }

    std::string name = geom->name();
    m_GeomList[name] = std::move(geom);
    return nullptr;
}

std::string *Simulation::ParseMuscle(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Muscle> muscle;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "MinettiAlexander"s)
    {
        muscle = std::make_unique<MAMuscle>();
        muscle->setSimulation(this);
        muscle->createAttributeMap(node->attributes);
        errorMessage = muscle->createFromAttributes();
    }
    else if (buf == "MinettiAlexanderComplete"s)
    {
        muscle = std::make_unique<MAMuscleComplete>();
        muscle->setSimulation(this);
        muscle->createAttributeMap(node->attributes);
        errorMessage = muscle->createFromAttributes();
    }
    else if (buf == "DampedSpring"s)
    {
        muscle = std::make_unique<DampedSpringMuscle>();
        muscle->setSimulation(this);
        muscle->createAttributeMap(node->attributes);
        errorMessage = muscle->createFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseMuscle Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }

    std::string name = muscle->name();
    m_MuscleList[name] = std::move(muscle);
    return nullptr;
}

std::string *Simulation::ParseStrap(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Strap> strap;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "TwoPoint"s)
    {
        strap = std::make_unique<TwoPointStrap>();
        strap->setSimulation(this);
        strap->createAttributeMap(node->attributes);
        errorMessage = strap->createFromAttributes();
    }
    else if (buf == "NPoint"s)
    {
        strap = std::make_unique<NPointStrap>();
        strap->setSimulation(this);
        strap->createAttributeMap(node->attributes);
        errorMessage = strap->createFromAttributes();
    }
    else if (buf == "CylinderWrap"s)
    {
        strap = std::make_unique<CylinderWrapStrap>();
        strap->setSimulation(this);
        strap->createAttributeMap(node->attributes);
        errorMessage = strap->createFromAttributes();
    }
    else if (buf == "TwoCylinderWrap"s)
    {
        strap = std::make_unique<TwoCylinderWrapStrap>();
        strap->setSimulation(this);
        strap->createAttributeMap(node->attributes);
        errorMessage = strap->createFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseStrap Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }

    std::string name = strap->name();
    m_StrapList[name] = std::move(strap);
    return nullptr;
}

std::string *Simulation::ParseFluidSac(const ParseXML::XMLElement *node)
{
    std::unique_ptr<FluidSac> fluidSac;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "IdealGas"s)
    {
        fluidSac = std::make_unique<FluidSacIdealGas>();
        fluidSac->setSimulation(this);
        fluidSac->createAttributeMap(node->attributes);
        errorMessage = fluidSac->createFromAttributes();
    }
    else if (buf == "Incompressible"s)
    {
        fluidSac = std::make_unique<FluidSacIncompressible>();
        fluidSac->setSimulation(this);
        fluidSac->createAttributeMap(node->attributes);
        errorMessage = fluidSac->createFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseFluidSac Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }

    std::string name = fluidSac->name();
    m_FluidSacList[name] = std::move(fluidSac);
    return nullptr;
}

std::string *Simulation::ParseDriver(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Driver> driver;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "Cyclic"s)
    {
        driver = std::make_unique<CyclicDriver>();
    }
    else if (buf == "Fixed"s)
    {
        driver = std::make_unique<FixedDriver>();
    }
    else if (buf == "StackedBoxcar"s)
    {
        driver = std::make_unique<StackedBoxcarDriver>();
    }
    else if (buf == "Step"s)
    {
        driver = std::make_unique<StepDriver>();
    }
    else if (buf == "Tegotae"s)
    {
        driver = std::make_unique<TegotaeDriver>();
    }
    else
    {
        setLastError("Simulation::ParseDriver Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    driver->setSimulation(this);
    driver->createAttributeMap(node->attributes);
    errorMessage = driver->createFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }

    std::string name = driver->name();
    m_DriverList[name] = std::move(driver);
    return nullptr;
}


std::string *Simulation::ParseDataTarget(const ParseXML::XMLElement *node)
{
    std::unique_ptr<DataTarget> dataTarget;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "Scalar"s)
    {
        dataTarget = std::make_unique<DataTargetScalar>();
    }
    else if (buf == "Quaternion"s)
    {
        dataTarget = std::make_unique<DataTargetQuaternion>();
    }
    else if (buf == "Vector"s)
    {
        dataTarget = std::make_unique<DataTargetVector>();
    }
    else
    {
        setLastError("Simulation::ParseDataTarget Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    dataTarget->setSimulation(this);
    dataTarget->createAttributeMap(node->attributes);
    errorMessage = dataTarget->createFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }

    std::string name = dataTarget->name();
    m_DataTargetList[name] = std::move(dataTarget);
    return nullptr;

}

std::string * Simulation::ParseReporter(const ParseXML::XMLElement * /*node*/)
{
//    char *buf;
//    Reporter *reporter;
//    THROWIFZERO(buf = DoXmlGetProp(cur, "Type"));

//    if (strcmp((const char *)buf, "Torque") == 0)
//    {
//        TorqueReporter *torqueReporter = new TorqueReporter();
//        torqueReporter->setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        torqueReporter->SetName((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "BodyID"));
//        torqueReporter->SetBody(m_BodyList[(const char *)buf]);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "MuscleID"));
//        torqueReporter->SetMuscle(m_MuscleList[(const char *)buf]);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "PivotPoint"));
//        torqueReporter->SetPivotPoint((const char *)buf);

//        buf = DoXmlGetProp(cur, "Axis");
//        if (buf)
//                torqueReporter->SetAxis((const char *)buf);

//        reporter = torqueReporter;
//    }

//    else if (strcmp((const char *)buf, "Position") == 0)
//    {
//        PositionReporter *positionReporter = new PositionReporter();
//        positionReporter->setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        positionReporter->SetName((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "BodyID"));
//        Body *bodyID = m_BodyList[(const char *)buf];
//        positionReporter->SetBody(bodyID);

//        buf = DoXmlGetProp(cur, "Position");
//        if (buf)
//            positionReporter->SetPosition((const char *)buf);

//        buf = DoXmlGetProp(cur, "Quaternion");
//        if (buf)
//            positionReporter->SetQuaternion((const char *)buf);

//        reporter = positionReporter;
//    }

//    else if (strcmp((const char *)buf, "SwingClearanceAbort") == 0)
//    {
//        SwingClearanceAbortReporter *swingClearanceAbortReporter = new SwingClearanceAbortReporter();
//        swingClearanceAbortReporter->setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        swingClearanceAbortReporter->SetName((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "BodyID"));
//        Body *bodyID = m_BodyList[(const char *)buf];
//        swingClearanceAbortReporter->SetBody(bodyID);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "Position"));
//        swingClearanceAbortReporter->SetPosition((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "HeightThreshold"));
//        swingClearanceAbortReporter->SetHeightThreshold(GSUtil::Double(buf));

//        THROWIFZERO(buf = DoXmlGetProp(cur, "VelocityThreshold"));
//        swingClearanceAbortReporter->SetVelocityThreshold(GSUtil::Double(buf));

//        // defaults to Z up
//        buf = DoXmlGetProp(cur, "UpAxis");
//        if (buf) swingClearanceAbortReporter->SetUpAxis(buf);

//        // optionally specify a direction axis for the velocity test
//        // gets normalised internally
//        buf = DoXmlGetProp(cur, "DirectionAxis");
//        if (buf) swingClearanceAbortReporter->SetDirectionAxis(buf);

//        reporter = swingClearanceAbortReporter;
//    }

//    else
//    {
//        throw __LINE__;
//    }

//    m_ReporterList[reporter->name()] = reporter;
    return nullptr;
}

std::string *Simulation::ParseController(const ParseXML::XMLElement *node)
{
    std::unique_ptr<Controller> controller;
    std::string buf = NamedObject::searchNames(node->attributes, "Type"s);
    std::string *errorMessage = nullptr;
    if (buf == "PIDMuscleLength"s)
    {
        controller = std::make_unique<PIDMuscleLength>();
    }
    else if (buf == "PIDTargetMatch"s)
    {
        controller = std::make_unique<PIDTargetMatch>();
    }
    else if (buf == "PIDErrorIn"s)
    {
        controller = std::make_unique<PIDErrorInController>();
    }
    else
    {
        setLastError("Simulation::ParseController Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    controller->setSimulation(this);
    controller->createAttributeMap(node->attributes);
    errorMessage = controller->createFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }

    std::string name = controller->name();
    m_ControllerList[name] = std::move(controller);
    return nullptr;
}


std::string * Simulation::ParseWarehouse(const ParseXML::XMLElement * /*node*/)
{
    return nullptr;
}

// function to produce a file of link kinematics in tab delimited format
// plus additional muscle activation information for use in gait warehousing

void Simulation::OutputWarehouse()
{
    if (m_OutputWarehouseAsText)
    {
        /* file format is \t separated and \n end of line
         *
         * numDrivers name0 name1 name2 ... numBodies name0 name1 name2...
         * time act0 act1 act2 ... x0 y0 z0 angle0 xaxis0 yaxis0 zaxis0 xv0 yv0 zv0 xav0 yav0 zav0 ...
         *
         */

        /* the first defined body is defined as its world coordinates and the rest are relative to the master body */

        // first time through output the column headings
        if (m_OutputWarehouseLastTime < 0)
        {
#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)
            m_OutputWarehouseFile.open(DataFile::ConvertUTF8ToWide(m_OutputWarehouseFilename));
#else
            m_OutputWarehouseFile.open(m_OutputWarehouseFilename);
#endif

            m_OutputWarehouseFile << m_DriverList.size();
            for (auto &&iter : m_DriverList) m_OutputWarehouseFile << "\t\"" << iter.second->name() << "\"";
            m_OutputWarehouseFile << "\t" << m_BodyList.size();
            m_OutputWarehouseFile << "\t" << m_BodyList[m_global->DistanceTravelledBodyIDName()]->name();
            for (auto &&iter : m_BodyList)
                if (iter.second->name() != m_global->DistanceTravelledBodyIDName()) m_OutputWarehouseFile << "\t\"" << iter.second->name() << "\"";
            m_OutputWarehouseFile << "\n";
        }

        m_OutputWarehouseLastTime = m_SimulationTime;
        // simulation time
        m_OutputWarehouseFile << m_SimulationTime;
        // driver activations
        for (auto &&iter : m_DriverList) m_OutputWarehouseFile << "\t" << iter.second->value();
        // output the root body (m_global->DistanceTravelledBodyIDName())
        Body *rootBody = m_BodyList[m_global->DistanceTravelledBodyIDName()].get();
        pgd::Vector pos, vel, avel;
        pgd::Quaternion quat;
        rootBody->GetRelativePosition(nullptr, &pos);
        rootBody->GetRelativeQuaternion(nullptr, &quat);
        rootBody->GetRelativeLinearVelocity(nullptr, &vel);
        rootBody->GetRelativeAngularVelocity(nullptr, &avel);
        double angle = QGetAngle(quat);
        pgd::Vector axis = QGetAxis(quat);
        m_OutputWarehouseFile << "\t" << pos.x << "\t" << pos.y << "\t" << pos.z;
        m_OutputWarehouseFile << "\t" << angle << "\t" << axis.x << "\t" << axis.y << "\t" << axis.z ;
        m_OutputWarehouseFile << "\t" << vel.x << "\t" << vel.y << "\t" << vel.z;
        m_OutputWarehouseFile << "\t" << avel.x << "\t" << avel.y << "\t" << avel.z;
        // and now the rest of the bodies
        for (auto &&iter : m_BodyList)
        {
            if (iter.second->name() != m_global->DistanceTravelledBodyIDName())
            {
                iter.second->GetRelativePosition(rootBody, &pos);
                iter.second->GetRelativeQuaternion(rootBody, &quat);
                iter.second->GetRelativeLinearVelocity(rootBody, &vel);
                iter.second->GetRelativeAngularVelocity(rootBody, &avel);
                angle = QGetAngle(quat);
                axis = QGetAxis(quat);
                m_OutputWarehouseFile << "\t" << pos.x << "\t" << pos.y << "\t" << pos.z;
                m_OutputWarehouseFile << "\t" << angle << "\t" << axis.x << "\t" << axis.y << "\t" << axis.z ;
                m_OutputWarehouseFile << "\t" << vel.x << "\t" << vel.y << "\t" << vel.z;
                m_OutputWarehouseFile << "\t" << avel.x << "\t" << avel.y << "\t" << avel.z;
            }
        }
        m_OutputWarehouseFile << "\n";
    }
    else
    {
        /* file format is binary
         *
         * int 0
         * int numDrivers int lenName0 name0 int lenName1 name1 ...
         * int numBodies int lenName0 name0 int lenName1 name1 ...
         * double time double act0 double act1 double act2 ...
         * double x0 double y0 double z0 double angle0 double xaxis0 double yaxis0 double zaxis0 double xv0 double yv0 double zv0 double xav0 double yav0 double zav0 ...
         *
         */

        /* the first defined body is defined as its world coordinates and the rest are relative to the master body */

        // first time through output the column headings
        if (m_OutputWarehouseLastTime < 0)
        {
#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)
            m_OutputWarehouseFile.open(DataFile::ConvertUTF8ToWide(m_OutputWarehouseFilename), std::ios::binary);
#else
            m_OutputWarehouseFile.open(m_OutputWarehouseFilename, std::ios::binary);
#endif
            GSUtil::BinaryOutput(m_OutputWarehouseFile, uint32_t(0));
            GSUtil::BinaryOutput(m_OutputWarehouseFile, uint32_t(m_DriverList.size()));
            for (auto &&iter : m_DriverList) GSUtil::BinaryOutput(m_OutputWarehouseFile, iter.second->name());
            GSUtil::BinaryOutput(m_OutputWarehouseFile, uint32_t(m_BodyList.size()));
            GSUtil::BinaryOutput(m_OutputWarehouseFile, m_BodyList[m_global->DistanceTravelledBodyIDName()]->name());
            for (auto &&iter : m_BodyList)
                if (iter.second->name() != m_global->DistanceTravelledBodyIDName()) GSUtil::BinaryOutput(m_OutputWarehouseFile, iter.second->name());
        }

        m_OutputWarehouseLastTime = m_SimulationTime;
        // simulation time
        GSUtil::BinaryOutput(m_OutputWarehouseFile, m_SimulationTime);
        // driver activations
        for (auto &&iter : m_DriverList) GSUtil::BinaryOutput(m_OutputWarehouseFile, iter.second->value());
        // output the root body (m_global->DistanceTravelledBodyIDName())
        Body *rootBody = m_BodyList[m_global->DistanceTravelledBodyIDName()].get();
        pgd::Vector pos, vel, avel;
        pgd::Quaternion quat;
        rootBody->GetRelativePosition(nullptr, &pos);
        rootBody->GetRelativeQuaternion(nullptr, &quat);
        rootBody->GetRelativeLinearVelocity(nullptr, &vel);
        rootBody->GetRelativeAngularVelocity(nullptr, &avel);
        double angle = QGetAngle(quat);
        pgd::Vector axis = QGetAxis(quat);
        GSUtil::BinaryOutput(m_OutputWarehouseFile, pos.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, pos.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, pos.z);
        GSUtil::BinaryOutput(m_OutputWarehouseFile, angle); GSUtil::BinaryOutput(m_OutputWarehouseFile, axis.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, axis.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, axis.z);
        GSUtil::BinaryOutput(m_OutputWarehouseFile, vel.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, vel.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, vel.z);
        GSUtil::BinaryOutput(m_OutputWarehouseFile, avel.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, avel.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, avel.z);
        // and now the rest of the bodies
        for (auto &&iter : m_BodyList)
        {
            if (iter.second->name() != m_global->DistanceTravelledBodyIDName())
            {
                iter.second->GetRelativePosition(rootBody, &pos);
                iter.second->GetRelativeQuaternion(rootBody, &quat);
                iter.second->GetRelativeLinearVelocity(rootBody, &vel);
                iter.second->GetRelativeAngularVelocity(rootBody, &avel);
                angle = QGetAngle(quat);
                axis = QGetAxis(quat);
                GSUtil::BinaryOutput(m_OutputWarehouseFile, pos.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, pos.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, pos.z);
                GSUtil::BinaryOutput(m_OutputWarehouseFile, angle); GSUtil::BinaryOutput(m_OutputWarehouseFile, axis.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, axis.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, axis.z);
                GSUtil::BinaryOutput(m_OutputWarehouseFile, vel.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, vel.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, vel.z);
                GSUtil::BinaryOutput(m_OutputWarehouseFile, avel.x); GSUtil::BinaryOutput(m_OutputWarehouseFile, avel.y); GSUtil::BinaryOutput(m_OutputWarehouseFile, avel.z);
            }
        }
    }
}

// output the simulation state in an XML format that can be re-read

void Simulation::OutputProgramState()
{
    m_parseXML.elementList()->clear();

    m_global->saveToAttributes(); m_parseXML.AddElement("GLOBAL"s, m_global->attributeMap());
    for (auto &&it : m_BodyList) { it.second->saveToAttributes(); m_parseXML.AddElement("BODY"s, it.second->attributeMap()); }
    for (auto &&it : m_MarkerList) { it.second->saveToAttributes(); m_parseXML.AddElement("MARKER"s, it.second->attributeMap()); }
    for (auto &&it : m_JointList) { it.second->saveToAttributes(); m_parseXML.AddElement("JOINT"s, it.second->attributeMap()); }
    for (auto &&it : m_GeomList) { it.second->saveToAttributes(); m_parseXML.AddElement("GEOM"s, it.second->attributeMap()); }
    for (auto &&it : m_StrapList) { it.second->saveToAttributes(); m_parseXML.AddElement("STRAP"s, it.second->attributeMap()); }
    for (auto &&it : m_MuscleList) { it.second->saveToAttributes(); m_parseXML.AddElement("MUSCLE"s, it.second->attributeMap()); }
    for (auto &&it : m_FluidSacList) { it.second->saveToAttributes(); m_parseXML.AddElement("FLUIDSAC"s, it.second->attributeMap()); }
    for (auto &&it : m_ReporterList) { it.second->saveToAttributes(); m_parseXML.AddElement("REPORTER"s, it.second->attributeMap()); }
    for (auto &&it : m_ControllerList) { it.second->saveToAttributes(); m_parseXML.AddElement("CONTROLLER"s, it.second->attributeMap()); }
    for (auto &&it : m_WarehouseList) { it.second->saveToAttributes(); m_parseXML.AddElement("WAREHOUSE"s, it.second->attributeMap()); }
    for (auto &&it : m_DriverList) { it.second->saveToAttributes(); m_parseXML.AddElement("DRIVER"s, it.second->attributeMap()); }
    for (auto &&it : m_DataTargetList) { it.second->saveToAttributes(); m_parseXML.AddElement("DATATARGET"s, it.second->attributeMap()); }

    // convert to string and output
    std::string xmlString = m_parseXML.SaveModel();
    DataFile outputFile;
    outputFile.SetRawData(xmlString.c_str(), xmlString.size());
    outputFile.WriteFile(m_OutputModelStateFile);
}

void Simulation::SetOutputModelStateFile(const std::string &filename)
{
    m_OutputModelStateFile = filename;
}

void Simulation::SetOutputWarehouseFile(const std::string &filename)
{
    if (filename.size() > 0)
    {
        if (m_OutputWarehouseFlag) m_OutputWarehouseFile.close();
        m_OutputWarehouseFilename = filename;
        m_OutputWarehouseFlag = true;
    }
    else
    {
        if (m_OutputWarehouseFlag) m_OutputWarehouseFile.close();
        m_OutputWarehouseFlag = false;
        m_OutputWarehouseFilename.empty();
    }
}

void Simulation::SetWarehouseFailDistanceAbort(double warehouseFailDistanceAbort)
{
    m_global->setWarehouseFailDistanceAbort(warehouseFailDistanceAbort);
}

void Simulation::SetGlobal(std::unique_ptr<Global> global)
{
    m_global = std::move(global);
    // set the global simulation parameters
    dWorldSetGravity(m_WorldID, m_global->Gravity().x, m_global->Gravity().y, m_global->Gravity().z);
    dWorldSetERP(m_WorldID, m_global->ERP());
    dWorldSetCFM(m_WorldID, m_global->CFM());
    dWorldSetContactMaxCorrectingVel(m_WorldID, m_global->ContactMaxCorrectingVel());
    dWorldSetContactSurfaceLayer(m_WorldID, m_global->ContactSurfaceLayer());
    dWorldSetDamping(m_WorldID, m_global->LinearDamping(), m_global->AngularDamping());
}

// add a warehouse from a file
void Simulation::AddWarehouse(const std::string &filename)
{
}

bool Simulation::ShouldQuit()
{
    if (m_global->TimeLimit() > 0)
        if (m_SimulationTime > m_global->TimeLimit()) return true;
    if (m_global->MechanicalEnergyLimit() > 0)
        if (m_MechanicalEnergy > m_global->MechanicalEnergyLimit()) return true;
    if (m_global->MetabolicEnergyLimit() > 0)
        if (m_MetabolicEnergy > m_global->MetabolicEnergyLimit()) return true;
    return false;
}

// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

void Simulation::NearCallback(void *data, dGeomID o1, dGeomID o2)
{
    int numc;
    Simulation *s = reinterpret_cast<Simulation *>(data);

    // exit without doing anything if the two bodies are connected by a joint
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    if (s->m_global->AllowConnectedCollisions() == false)
    {
        if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact)) return;
    }

    if (s->m_global->AllowInternalCollisions() == false)
    {
        if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetGeomLocation() == reinterpret_cast<Geom *>(dGeomGetData(o2))->GetGeomLocation()) return;
    }

    std::unique_ptr<dContact[]> contact = std::make_unique<dContact[]>(size_t(s->m_MaxContacts)); // up to m_MaxContacts contacts per box-box
    double cfm = MAX(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactSoftCFM(),
                     reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactSoftCFM());
    double erp = MIN(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactSoftERP(),
                     reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactSoftERP());
    double mu = MIN(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactMu(),
                    reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactMu());
    double bounce = MAX(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactBounce(),
                        reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactBounce());
    for (size_t i = 0; i < size_t(s->m_MaxContacts); i++)
    {
        contact[i].surface.mode = dContactApprox1;
        contact[i].surface.mu = mu;
        if (bounce >= 0)
        {
            contact[i].surface.bounce = bounce;
            contact[i].surface.mode += dContactBounce;
        }
        if (cfm >= 0)
        {
            contact[i].surface.soft_cfm = cfm;
            contact[i].surface.mode += dContactSoftCFM;
        }
        if (erp <= 1)
        {
            contact[i].surface.soft_erp = erp;
            contact[i].surface.mode += dContactSoftERP;
        }
    }
    numc = dCollide(o1, o2, s->m_MaxContacts, &contact[0].geom, sizeof(dContact));
    if (numc)
    {
        for (size_t i = 0; i < size_t(numc); i++)
        {
            if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetAbort()) s->SetContactAbort(true);
            if (reinterpret_cast<Geom *>(dGeomGetData(o2))->GetAbort()) s->SetContactAbort(true);
            dJointID c;
            if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetAdhesion() == false && reinterpret_cast<Geom *>(dGeomGetData(o2))->GetAdhesion() == false)
            {
                c = dJointCreateContact(s->m_WorldID, s->m_ContactGroup, &contact[i]);
                dJointAttach(c, b1, b2);
                std::unique_ptr<Contact> myContact = std::make_unique<Contact>();
                myContact->setSimulation(s);
                dJointSetFeedback(c, myContact->GetJointFeedback());
                myContact->SetJointID(c);
                std::copy_n(contact[i].geom.pos, dV3E__MAX, myContact->GetContactPosition());
                // only add the contact information once
                // and add it to the non-environment geom
                if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetGeomLocation() == Geom::environment)
                    reinterpret_cast<Geom *>(dGeomGetData(o2))->AddContact(myContact.get());
                else
                    reinterpret_cast<Geom *>(dGeomGetData(o1))->AddContact(myContact.get());
                s->m_ContactList.push_back(std::move(myContact));
            }
            else
            {
                // FIX ME adhesive joints are added permanently and forces cannot be measured
                c = dJointCreateBall(s->m_WorldID, nullptr);
                dJointAttach(c, b1, b2);
                dJointSetBallAnchor(c, contact[i].geom.pos[0], contact[i].geom.pos[1], contact[i].geom.pos[2]);
            }
        }
    }
}

Body *Simulation::GetBody(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_BodyList.find(name);
    if (iter != m_BodyList.end()) return iter->second.get();
    return nullptr;
}

Joint *Simulation::GetJoint(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_JointList.find(name);
    if (iter != m_JointList.end()) return iter->second.get();
    return nullptr;
}

Geom *Simulation::GetGeom(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_GeomList.find(name);
    if (iter != m_GeomList.end()) return iter->second.get();
    return nullptr;
}

Muscle *Simulation::GetMuscle(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_MuscleList.find(name);
    if (iter != m_MuscleList.end()) return iter->second.get();
    return nullptr;
}

Strap *Simulation::GetStrap(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_StrapList.find(name);
    if (iter != m_StrapList.end()) return iter->second.get();
    return nullptr;
}

FluidSac *Simulation::GetFluidSac(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_FluidSacList.find(name);
    if (iter != m_FluidSacList.end()) return iter->second.get();
    return nullptr;
}

Driver *Simulation::GetDriver(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_DriverList.find(name);
    if (iter != m_DriverList.end()) return iter->second.get();
    return nullptr;
}

DataTarget *Simulation::GetDataTarget(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_DataTargetList.find(name);
    if (iter != m_DataTargetList.end()) return iter->second.get();
    return nullptr;
}

Marker *Simulation::GetMarker(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_MarkerList.find(name);
    if (iter != m_MarkerList.end()) return iter->second.get();
    return nullptr;
}

Reporter *Simulation::GetReporter(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_ReporterList.find(name);
    if (iter != m_ReporterList.end()) return iter->second.get();
    return nullptr;
}

Controller *Simulation::GetController(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_ControllerList.find(name);
    if (iter != m_ControllerList.end()) return iter->second.get();
    return nullptr;
}

Warehouse *Simulation::GetWarehouse(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_WarehouseList.find(name);
    if (iter != m_WarehouseList.end()) return iter->second.get();
    return nullptr;
}

Global *Simulation::GetGlobal()
{
    return m_global.get();
}

void Simulation::DumpObjects()
{
    for (auto &&it : m_BodyList) DumpObject(it.second.get());
    for (auto &&it : m_MarkerList) DumpObject(it.second.get());
    for (auto &&it : m_JointList) DumpObject(it.second.get());
    for (auto &&it : m_GeomList) DumpObject(it.second.get());
    for (auto &&it : m_FluidSacList) DumpObject(it.second.get());
    for (auto &&it : m_DriverList) DumpObject(it.second.get());
    for (auto &&it : m_DataTargetList) DumpObject(it.second.get());
    for (auto &&it : m_ReporterList) DumpObject(it.second.get());
    for (auto &&it : m_ControllerList) DumpObject(it.second.get());
    for (auto &&it : m_WarehouseList) DumpObject(it.second.get());
    for (auto &&it : m_MuscleList)
    {
        DumpObject(it.second.get());
        DumpObject(it.second->GetStrap());
    }
}

void Simulation::DumpObject(NamedObject *namedObject)
{
    if (namedObject->getDump())
    {
        if (namedObject->getFirstDump())
        {
            std::ofstream output;
#if defined _WIN32 && defined _MSC_VER // required because windows and visual studio require wstring for full filename support
            output.open(DataFile::ConvertUTF8ToWide(namedObject->name() + m_dumpExtension));
#else
            output.open(namedObject->name() + m_dumpExtension);
#endif
            m_dumpFileStreams[namedObject->name()] = std::move(output);
        }
        auto fileIt = m_dumpFileStreams.find(namedObject->name());
        if (fileIt != m_dumpFileStreams.end()) fileIt->second << namedObject->dump();
    }
}

std::vector<std::string> Simulation::GetNameList() const
{
    std::vector<std::string> output;
    size_t size = m_BodyList.size() +
            m_JointList.size() +
            m_GeomList.size() +
            m_MuscleList.size() +
            m_StrapList.size() +
            m_FluidSacList.size() +
            m_DriverList.size() +
            m_DataTargetList.size() +
            m_MarkerList.size() +
            m_ReporterList.size() +
            m_ControllerList.size() +
            m_WarehouseList.size();
    output.reserve(size);
    for (auto &&it : m_BodyList) output.push_back(it.first);
    for (auto &&it : m_JointList) output.push_back(it.first);
    for (auto &&it : m_GeomList) output.push_back(it.first);
    for (auto &&it : m_MuscleList) output.push_back(it.first);
    for (auto &&it : m_StrapList) output.push_back(it.first);
    for (auto &&it : m_FluidSacList) output.push_back(it.first);
    for (auto &&it : m_DriverList) output.push_back(it.first);
    for (auto &&it : m_DataTargetList) output.push_back(it.first);
    for (auto &&it : m_MarkerList) output.push_back(it.first);
    for (auto &&it : m_ReporterList) output.push_back(it.first);
    for (auto &&it : m_ControllerList) output.push_back(it.first);
    for (auto &&it : m_WarehouseList) output.push_back(it.first);
    return output;
}

std::set<std::string> Simulation::GetNameSet() const
{
    std::set<std::string> output;
    for (auto &&it : m_BodyList) output.insert(it.first);
    for (auto &&it : m_JointList) output.insert(it.first);
    for (auto &&it : m_GeomList) output.insert(it.first);
    for (auto &&it : m_MuscleList) output.insert(it.first);
    for (auto &&it : m_StrapList) output.insert(it.first);
    for (auto &&it : m_FluidSacList) output.insert(it.first);
    for (auto &&it : m_DriverList) output.insert(it.first);
    for (auto &&it : m_DataTargetList) output.insert(it.first);
    for (auto &&it : m_MarkerList) output.insert(it.first);
    for (auto &&it : m_ReporterList) output.insert(it.first);
    for (auto &&it : m_ControllerList) output.insert(it.first);
    for (auto &&it : m_WarehouseList) output.insert(it.first);
    return output;
}

NamedObject *Simulation::GetNamedObject(const std::string &name) const
{
    auto BodyListIt = m_BodyList.find(name); if (BodyListIt != m_BodyList.end()) return BodyListIt->second.get();
    auto JointListIt = m_JointList.find(name); if (JointListIt != m_JointList.end()) return JointListIt->second.get();
    auto GeomListIt = m_GeomList.find(name); if (GeomListIt != m_GeomList.end()) return GeomListIt->second.get();
    auto MuscleListIt = m_MuscleList.find(name); if (MuscleListIt != m_MuscleList.end()) return MuscleListIt->second.get();
    auto StrapListIt = m_StrapList.find(name); if (StrapListIt != m_StrapList.end()) return StrapListIt->second.get();
    auto FluidSacListIt = m_FluidSacList.find(name); if (FluidSacListIt != m_FluidSacList.end()) return FluidSacListIt->second.get();
    auto DriverListIt = m_DriverList.find(name); if (DriverListIt != m_DriverList.end()) return DriverListIt->second.get();
    auto DataTargetListIt = m_DataTargetList.find(name); if (DataTargetListIt != m_DataTargetList.end()) return DataTargetListIt->second.get();
    auto MarkerListIt = m_MarkerList.find(name); if (MarkerListIt != m_MarkerList.end()) return MarkerListIt->second.get();
    auto ReporterListIt = m_ReporterList.find(name); if (ReporterListIt != m_ReporterList.end()) return ReporterListIt->second.get();
    auto ControllerListIt = m_ControllerList.find(name); if (ControllerListIt != m_ControllerList.end()) return ControllerListIt->second.get();
    auto WarehouseListIt = m_WarehouseList.find(name); if (WarehouseListIt != m_WarehouseList.end()) return WarehouseListIt->second.get();
    return nullptr;
}

std::map<std::string, Drivable *> Simulation::GetDrivableList() const
{
    std::map<std::string, Drivable *> output;
    for (auto &&it : m_MuscleList) output[it.first] = it.second.get();
    for (auto &&it : m_ControllerList) output[it.first] = it.second.get();
    return output;
}


