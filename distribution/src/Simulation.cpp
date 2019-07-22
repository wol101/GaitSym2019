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
#include "PositionReporter.h"
#include "UniversalJoint.h"
#include "PIDMuscleLength.h"
#include "Controller.h"
#include "SwingClearanceAbortReporter.h"
#include "AMotorJoint.h"
#include "SliderJoint.h"
#include "BoxGeom.h"
#include "StackedBoxCarDriver.h"
#include "PIDTargetMatch.h"
#include "Warehouse.h"
#include "FixedDriver.h"
#include "PIDErrorInController.h"
#include "TegotaeDriver.h"

#ifdef USE_QT
#include "SimulationWindow.h"
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
    dSetMessageHandler(nullptr);

    // get rid of all those memory alloactions

    for (std::map<std::string, Body *>::const_iterator iter1 = m_BodyList.begin(); iter1 != m_BodyList.end(); iter1++) delete iter1->second;
    for (std::map<std::string, Joint *>::const_iterator iter2 = m_JointList.begin(); iter2 != m_JointList.end(); iter2++) delete iter2->second;
    for (std::map<std::string, Muscle *>::const_iterator iter3 = m_MuscleList.begin(); iter3 != m_MuscleList.end(); iter3++) delete iter3->second;
    for (std::map<std::string, Driver *>::const_iterator iter4 = m_DriverList.begin(); iter4 != m_DriverList.end(); iter4++) delete iter4->second;
    for (std::map<std::string, DataTarget *>::const_iterator iter5 = m_DataTargetList.begin(); iter5 != m_DataTargetList.end(); iter5++) delete iter5->second;
    for (std::map<std::string, Geom *>::const_iterator iter6 = m_GeomList.begin(); iter6 != m_GeomList.end(); iter6++) delete iter6->second;
    for (std::map<std::string, Marker *>::const_iterator iter7 = m_MarkerList.begin(); iter7 != m_MarkerList.end(); iter7++) delete iter7->second;
    for (std::map<std::string, Reporter *>::const_iterator iter8 = m_ReporterList.begin(); iter8 != m_ReporterList.end(); iter8++) delete iter8->second;
    for (std::map<std::string, Controller *>::const_iterator iter9 = m_ControllerList.begin(); iter9 != m_ControllerList.end(); iter9++) delete iter9->second;
    for (std::map<std::string, Warehouse *>::const_iterator iter10 = m_WarehouseList.begin(); iter10 != m_WarehouseList.end(); iter10++) delete iter10->second;
    for (std::map<std::string, Strap *>::const_iterator iter11 = m_StrapList.begin(); iter11 != m_StrapList.end(); iter11++) delete iter11->second;
    for (std::map<std::string, FluidSac *>::const_iterator iter12 = m_FluidSacList.begin(); iter12 != m_FluidSacList.end(); iter12++) delete iter12->second;

    // destroy the ODE world
    dJointGroupDestroy(m_ContactGroup);
    dSpaceDestroy(m_SpaceID);
    dWorldDestroy(m_WorldID);
    dCloseODE();

    // delete the rest of the allocated memory
    for (unsigned int c = 0; c < m_ContactList.size(); c++) delete m_ContactList[c];

    // close any open files
    if (m_OutputWarehouseFlag) m_OutputWarehouseFile.close();
}

//----------------------------------------------------------------------------
std::string *Simulation::LoadModel(const char *buffer, size_t length)
{
    std::string *ptr = m_parseXML.LoadModel(buffer, length, "GAITSYM2019");
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
    for (auto it :  m_JointList) it.second->LateInitialisation();
    // then the bodies are moved to their starting poses
    for (auto it : m_BodyList) it.second->LateInitialisation();
    // and we recalculate the dynamic items with the new muscle positions
    for (auto it :  m_MuscleList) it.second->LateInitialisation();
    for (auto it : m_FluidSacList) it.second->LateInitialisation();

    m_DistanceTravelledBodyID = m_BodyList[m_global.DistanceTravelledBodyIDName()];
    if (m_DistanceTravelledBodyID == nullptr)
    {
        setLastError("Error parsing XML file: DistanceTravelledBodyIDName not found - \""s + m_global.DistanceTravelledBodyIDName() +"\""s);
        return lastErrorPtr();
    }

    // for the time being just set the current warehouse to the first one in the list
    if (m_global.CurrentWarehouseFile().length() == 0 && m_WarehouseList.size() > 0) m_global.setCurrentWarehouseFile(m_WarehouseList.begin()->first);

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
    if (m_global.fitnessType() != Global::DistanceTravelled)
    {
        if (m_global.fitnessType() == Global::ClosestWarehouse)
        {
        }
        else if (m_global.fitnessType() == Global::KinematicMatch || m_global.fitnessType() == Global::KinematicMatchMiniMax)
        {

            double minScore = DBL_MAX;
            double matchScore;
            for (std::map<std::string, DataTarget *>::const_iterator iter3 = m_DataTargetList.begin(); iter3 != m_DataTargetList.end(); iter3++)
            {
                int lastIndex = iter3->second->GetLastMatchIndex();
                int index = iter3->second->TargetMatch(m_SimulationTime, m_global.StepSize() * 0.50000000001);
                // on rare occasions because of rounding we may get two matches we can check this using the lastIndex since this is the only palce where a match is requested
                if (index != -1 && index != lastIndex) // since step size is much smaller than the interval between targets (probably), this should get called exactly once per target time defintion
                {
                    matchScore = iter3->second->GetMatchValue(index);
                    m_KinematicMatchFitness += matchScore;
                    if (matchScore < minScore)
                        minScore = matchScore;
                }
            }
            if (minScore < DBL_MAX)
                m_KinematicMatchMiniMaxFitness += minScore;
        }
        else if (m_global.fitnessType() == Global::KinematicMatchContinuous || m_global.fitnessType() == Global::KinematicMatchContinuousMiniMax)
        {

            double minScore = DBL_MAX;
            double matchScore;
            for (std::map<std::string, DataTarget *>::const_iterator iter3 = m_DataTargetList.begin(); iter3 != m_DataTargetList.end(); iter3++)
            {
                matchScore = iter3->second->GetMatchValue(m_SimulationTime);
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
    for (unsigned int c = 0; c < m_ContactList.size(); c++) delete m_ContactList[c];
    m_ContactList.clear();
    for (std::map<std::string, Geom *>::const_iterator GeomIter = m_GeomList.begin(); GeomIter != m_GeomList.end(); GeomIter++) GeomIter->second->ClearContacts();
    dSpaceCollide(m_SpaceID, this, &NearCallback);

    bool activationsDone = false;

    // update the muscles
    double tension;
    std::vector<PointForce *> *pointForceList;
    PointForce *pointForce;
    for (std::map<std::string, Muscle *>::const_iterator iter1 = m_MuscleList.begin(); iter1 != m_MuscleList.end(); )
    {
        if (activationsDone == false) iter1->second->SumDrivers(m_SimulationTime);
        iter1->second->CalculateStrap(m_SimulationTime);
        iter1->second->SetActivation(iter1->second->GetCurrentDriverSum(), m_global.StepSize());

        // check for breaking strain
        DampedSpringMuscle *dampedSpringMuscle = dynamic_cast<DampedSpringMuscle *>(iter1->second);
        if (dampedSpringMuscle)
        {
            if (dampedSpringMuscle->ShouldBreak())
            {
                iter1 = m_MuscleList.erase(iter1); // erase returns the next iterator [but m_MuscleList.erase(iter1++) would also work and is compatible with older C++ compilers]
                delete dampedSpringMuscle;
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
            pointForce = (*pointForceList)[i];
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
    for (std::map<std::string, Joint *>::const_iterator jointIter = m_JointList.begin(); jointIter != m_JointList.end(); jointIter++) jointIter->second->Update();

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
    switch (m_global.stepType())
    {
    case Global::WorldStep:
        dWorldStep(m_WorldID, m_global.StepSize());
        break;

    case Global::QuickStep:
        dWorldQuickStep(m_WorldID, m_global.StepSize());
        break;
    }

    // update the time counter
    m_SimulationTime += m_global.StepSize();

    // update the step counter
    m_StepCount++;

    // calculate the energies
    for (std::map<std::string, Muscle *>::const_iterator iter1 = m_MuscleList.begin(); iter1 != m_MuscleList.end(); iter1++)
    {
        m_MechanicalEnergy += iter1->second->GetPower() * m_global.StepSize();
        m_MetabolicEnergy += iter1->second->GetMetabolicPower() * m_global.StepSize();
    }
    m_MetabolicEnergy += m_global.BMR() * m_global.StepSize();

    // update any contact force dependent drivers (because only after the simulation is the force valid
    // update the footprint indicator
    if (m_ContactList.size() > 0)
    {
        for (std::map<std::string, Driver *>::const_iterator it = m_DriverList.begin(); it != m_DriverList.end(); it++)
        {
            TegotaeDriver *tegotaeDriver = dynamic_cast<TegotaeDriver *>(it->second);
            if (tegotaeDriver) tegotaeDriver->UpdateReactionForce();
        }
    }

    // all reporting is done after a simulation step

    Dump();

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
    for (std::map<std::string, Body *>::const_iterator iter1 = m_BodyList.begin(); iter1 != m_BodyList.end(); iter1++)
    {
        p = iter1->second->TestLimits();
        switch (p)
        {
        case Body::WithinLimits:
            break;

        case Body::XPosError:
        case Body::YPosError:
        case Body::ZPosError:
#if defined(USE_QT)
            ss << "Failed due to position error " << p << " in: " << iter1->second->GetName();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to position error " << p << " in: " << iter1->second->GetName() << "\n";
            return true;

        case Body::XVelError:
        case Body::YVelError:
        case Body::ZVelError:
#if defined(USE_QT)
            ss << "Failed due to velocity error " << p << " in: " << iter1->second->GetName();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to velocity error " << p << " in: " << iter1->second->GetName() << "\n";
            return true;

        case Body::NumericalError:
#if defined(USE_QT)
            ss << "Failed due to numerical error " << p << " in: " << iter1->second->GetName();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to numerical error " << p << " in: " << iter1->second->GetName() << "\n";
            return true;
        }
    }

    std::map<std::string, Joint *>::const_iterator iter3;
    HingeJoint *j;
    FixedJoint *f;
    int t;
    for (iter3 = m_JointList.begin(); iter3 != m_JointList.end(); iter3++)
    {
        j = dynamic_cast<HingeJoint *>(iter3->second);
        if (j)
        {
            t = j->TestLimits();
            if (t < 0)
            {
#if defined(USE_QT)
                ss << __FILE__ << "Failed due to LoStopTorqueLimit error in: " << iter3->second->GetName();
                if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
                std::cerr << "Failed due to LoStopTorqueLimit error in: " << iter3->second->GetName() << "\n";
                return true;
            }
            else if (t > 0)
            {
#if defined(USE_QT)
                ss << __FILE__ << "Failed due to HiStopTorqueLimit error in: " << iter3->second->GetName();
                if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
                std::cerr << "Failed due to HiStopTorqueLimit error in: " << iter3->second->GetName() << "\n";
                return true;
            }
        }

        f = dynamic_cast<FixedJoint *>(iter3->second);
        if (f)
        {
            if (f->CheckStressAbort())
            {
#if defined(USE_QT)
                ss << __FILE__ << "Failed due to stress limit error in: " << iter3->second->GetName() << " " << f->GetLowPassMinStress() << " " << f->GetLowPassMaxStress();
                if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
                std::cerr << "Failed due to stress limit error in: " << iter3->second->GetName() << " " << f->GetLowPassMinStress() << " " << f->GetLowPassMaxStress() << "\n";
                return true;
            }
        }
    }

    // and test the reporters for stop conditions
    std::map<std::string, Reporter *>::const_iterator ReporterIter;
    for (ReporterIter = m_ReporterList.begin(); ReporterIter != m_ReporterList.end(); ReporterIter++)
    {
        if (ReporterIter->second->ShouldAbort())
        {
#if defined(USE_QT)
            ss << __FILE__ << "Failed due to Reporter Abort in: " << ReporterIter->second->GetName();
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to Reporter Abort in: " << ReporterIter->second->GetName() << "\n";
            return true;
        }
    }

    // test for WarehouseFailDistanceAbort if set
    if (m_global.WarehouseFailDistanceAbort() > 0 && m_WarehouseList.size() > 0 && m_global.fitnessType() != Global::ClosestWarehouse)
    {
        if (m_WarehouseDistance > m_global.WarehouseFailDistanceAbort())
        {
#if defined(USE_QT)
            ss << __FILE__ << "Failed due to >WarehouseFailDistanceAbort. m_global.WarehouseFailDistanceAbort()=" << m_global.WarehouseFailDistanceAbort() << " WarehouseDistance = " << m_WarehouseDistance;
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to >WarehouseFailDistanceAbort. m_global.WarehouseFailDistanceAbort()=" << m_global.WarehouseFailDistanceAbort() << " WarehouseDistance = " << m_WarehouseDistance << "\n";
            return true;
        }
    }
    else if (m_global.WarehouseFailDistanceAbort() < 0 && m_WarehouseList.size() > 0 && m_global.fitnessType() != Global::ClosestWarehouse && m_SimulationTime > 0)
    {
        if (m_WarehouseDistance < std::fabs(m_global.WarehouseFailDistanceAbort()))
        {
#if defined(USE_QT)
            ss << __FILE__ << "Failed due to <WarehouseFailDistanceAbort. m_global.WarehouseFailDistanceAbort()=" << m_global.WarehouseFailDistanceAbort() << " WarehouseDistance = " << m_WarehouseDistance;
            if (m_MainWindow) m_MainWindow->log(ss.str().c_str());
#endif
            std::cerr << "Failed due to <WarehouseFailDistanceAbort. m_global.WarehouseFailDistanceAbort()=" << m_global.WarehouseFailDistanceAbort() << " WarehouseDistance = " << m_WarehouseDistance << "\n";
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
    switch (m_global.fitnessType())
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

    case Global::ClosestWarehouse:
        return m_ClosestWarehouseFitness;
    }
    return 0;
}

std::string *Simulation::ParseGlobal(const ParseXML::XMLElement *node)
{
    Global global;
    global.setSimulation(this);
    global.CreateAttributeMap(node->names, node->values);
    std::string *errorMessage = global.CreateFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        return lastErrorPtr();
    }
    this->SetGlobal(global);
    return nullptr;
}

std::string *Simulation::ParseBody(const ParseXML::XMLElement *node)
{
    Body *body = new Body(m_WorldID);
    body->setSimulation(this);
    body->CreateAttributeMap(node->names, node->values);
    std::string *errorMessage = body->CreateFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        delete body;
        return lastErrorPtr();
    }
    m_BodyList[body->GetName()] = body;
    return nullptr;
}

std::string *Simulation::ParseMarker(const ParseXML::XMLElement *node)
{
    Marker *marker = new Marker(nullptr);
    marker->setSimulation(this);
    marker->CreateAttributeMap(node->names, node->values);
    std::string *errorMessage = marker->CreateFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        delete marker;
        return lastErrorPtr();
    }
    m_MarkerList[marker->GetName()] = marker;
    return nullptr;
}

std::string *Simulation::ParseJoint(const ParseXML::XMLElement *node)
{
    Joint *joint = nullptr;
    std::string buf = NamedObject::SearchNames(node->names, node->values, "Type"s, false);
    std::string *errorMessage = nullptr;
    if (buf == "Hinge"s)
    {
        HingeJoint *hingeJoint = new HingeJoint(m_WorldID);
        joint = hingeJoint;
        joint->setSimulation(this);
        joint->CreateAttributeMap(node->names, node->values);
        errorMessage = joint->CreateFromAttributes();
    }
    else if (buf == "Ball"s)
    {
        BallJoint *ballJoint = new BallJoint(m_WorldID, BallJoint::NoStops);
        joint = ballJoint;
        joint->setSimulation(this);
        joint->CreateAttributeMap(node->names, node->values);
        errorMessage = joint->CreateFromAttributes();
    }
    else if (buf == "Fixed"s)
    {
        FixedJoint *fixedJoint = new FixedJoint(m_WorldID);
        joint = fixedJoint;
        joint->setSimulation(this);
        joint->CreateAttributeMap(node->names, node->values);
        errorMessage = joint->CreateFromAttributes();
    }
    else if (buf == "FloatingHinge"s)
    {
        FloatingHingeJoint *floatingHingeJoint = new FloatingHingeJoint(m_WorldID);
        joint = floatingHingeJoint;
        joint->setSimulation(this);
        joint->CreateAttributeMap(node->names, node->values);
        errorMessage = joint->CreateFromAttributes();
    }
    else if (buf == "Universal"s)
    {
        UniversalJoint *universalJoint = new UniversalJoint(m_WorldID);
        joint = universalJoint;
        joint->setSimulation(this);
        joint->CreateAttributeMap(node->names, node->values);
        errorMessage = joint->CreateFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseJoint Type=\"" + buf + "\" not recognised");
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        if (joint) delete joint;
        return lastErrorPtr();
    }

    m_JointList[joint->GetName()] = joint;
    return nullptr;
}

std::string *Simulation::ParseGeom(const ParseXML::XMLElement *node)
{
    Geom *geom = nullptr;
    std::string buf = NamedObject::SearchNames(node->names, node->values, "Type"s, false);
    std::string *errorMessage = nullptr;
    if (buf == "Box"s)
    {
        BoxGeom *boxGeom = new BoxGeom(m_SpaceID, 1.0, 1.0, 1.0);
        boxGeom->setSimulation(this);
        boxGeom->CreateAttributeMap(node->names, node->values);
        errorMessage = boxGeom->CreateFromAttributes();
        geom = boxGeom;
    }
    else if (buf == "CappedCylinder"s)
    {
        CappedCylinderGeom *cappedCylinderGeom = new CappedCylinderGeom(m_SpaceID, 1.0, 1.0);
        cappedCylinderGeom->setSimulation(this);
        cappedCylinderGeom->CreateAttributeMap(node->names, node->values);
        errorMessage = cappedCylinderGeom->CreateFromAttributes();
        geom = cappedCylinderGeom;
    }
    else if (buf == "Plane"s)
    {
        PlaneGeom *planeGeom = new PlaneGeom(m_SpaceID, 10.0, 0.0, 1.0, 0.0);
        planeGeom->setSimulation(this);
        planeGeom->CreateAttributeMap(node->names, node->values);
        errorMessage = planeGeom->CreateFromAttributes();
        geom = planeGeom;
    }
    else if (buf == "Sphere"s)
    {
        SphereGeom *sphereGeom = new SphereGeom(m_SpaceID, 1.0);
        sphereGeom->setSimulation(this);
        sphereGeom->CreateAttributeMap(node->names, node->values);
        errorMessage = sphereGeom->CreateFromAttributes();
        geom = sphereGeom;
    }
    else
    {
        setLastError("Simulation::ParseGeom Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        if (geom) delete geom;
        return lastErrorPtr();
    }

    m_GeomList[geom->GetName()] = geom;
    return nullptr;
}

std::string *Simulation::ParseMuscle(const ParseXML::XMLElement *node)
{
    Muscle *muscle = nullptr;
    std::string buf = NamedObject::SearchNames(node->names, node->values, "Type"s, false);
    std::string *errorMessage = nullptr;
    if (buf == "MinettiAlexander"s)
    {
        MAMuscle *maMuscle = new MAMuscle();
        muscle = maMuscle;
        muscle->setSimulation(this);
        muscle->CreateAttributeMap(node->names, node->values);
        errorMessage = muscle->CreateFromAttributes();
    }
    else if (buf == "MinettiAlexanderComplete"s)
    {
        MAMuscleComplete *maMuscleComplete = new MAMuscleComplete();
        muscle = maMuscleComplete;
        muscle->setSimulation(this);
        muscle->CreateAttributeMap(node->names, node->values);
        errorMessage = muscle->CreateFromAttributes();
    }
    else if (buf == "DampedSpring"s)
    {
        DampedSpringMuscle *dampedSpringMuscle = new DampedSpringMuscle();
        muscle = dampedSpringMuscle;
        muscle->setSimulation(this);
        muscle->CreateAttributeMap(node->names, node->values);
        errorMessage = muscle->CreateFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseMuscle Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        if (muscle) delete muscle;
        return lastErrorPtr();
    }

    m_MuscleList[muscle->GetName()] = muscle;
    return nullptr;
}

std::string *Simulation::ParseStrap(const ParseXML::XMLElement *node)
{
    Strap *strap = nullptr;
    std::string buf = NamedObject::SearchNames(node->names, node->values, "Type"s, false);
    std::string *errorMessage = nullptr;
    if (buf == "TwoPoint"s)
    {
        TwoPointStrap *twoPointStrap = new TwoPointStrap();
        strap = twoPointStrap;
        strap->setSimulation(this);
        strap->CreateAttributeMap(node->names, node->values);
        errorMessage = strap->CreateFromAttributes();
    }
    else if (buf == "NPoint"s)
    {
        NPointStrap *nPointStrap = new NPointStrap();
        strap = nPointStrap;
        strap->setSimulation(this);
        strap->CreateAttributeMap(node->names, node->values);
        errorMessage = strap->CreateFromAttributes();
    }
    else if (buf == "CylinderWrap"s)
    {
        CylinderWrapStrap *cylinderWrapStrap = new CylinderWrapStrap();
        strap = cylinderWrapStrap;
        strap->setSimulation(this);
        strap->CreateAttributeMap(node->names, node->values);
        errorMessage = strap->CreateFromAttributes();
    }
    else if (buf == "TwoCylinderWrap"s)
    {
        TwoCylinderWrapStrap *twoCylinderWrapStrap = new TwoCylinderWrapStrap();
        strap = twoCylinderWrapStrap;
        strap->setSimulation(this);
        strap->CreateAttributeMap(node->names, node->values);
        errorMessage = strap->CreateFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseStrap Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        if (strap) delete strap;
        return lastErrorPtr();
    }

    m_StrapList[strap->GetName()] = strap;
    return nullptr;
}

std::string *Simulation::ParseFluidSac(const ParseXML::XMLElement *node)
{
    FluidSac *fluidSac = nullptr;
    std::string buf = NamedObject::SearchNames(node->names, node->values, "Type"s, false);
    std::string *errorMessage = nullptr;
    if (buf == "IdealGas"s)
    {
        FluidSacIdealGas *idealGas = new FluidSacIdealGas();
        fluidSac = idealGas;
        fluidSac->setSimulation(this);
        fluidSac->CreateAttributeMap(node->names, node->values);
        errorMessage = fluidSac->CreateFromAttributes();
    }
    else if (buf == "Incompressible"s)
    {
        FluidSacIncompressible *incompressible = new FluidSacIncompressible();
        fluidSac = incompressible;
        fluidSac->setSimulation(this);
        fluidSac->CreateAttributeMap(node->names, node->values);
        errorMessage = fluidSac->CreateFromAttributes();
    }
    else
    {
        setLastError("Simulation::ParseFluidSac Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    if (errorMessage)
    {
        setLastError(*errorMessage);
        if (fluidSac) delete fluidSac;
        return lastErrorPtr();
    }

    m_FluidSacList[fluidSac->GetName()] = fluidSac;
    return nullptr;
}

std::string * Simulation::ParseDriver(const ParseXML::XMLElement *node)
{
    Driver *driver = nullptr;
    std::string buf = NamedObject::SearchNames(node->names, node->values, "Type"s, false);
    std::string *errorMessage = nullptr;
    if (buf == "Cyclic"s)
    {
        driver = new CyclicDriver();
    }
    else if (buf == "Fixed"s)
    {
        driver = new FixedDriver();;
    }
    else if (buf == "StackedBoxcar"s)
    {
        driver = new StackedBoxcarDriver();;
    }
    else if (buf == "Step"s)
    {
        driver = new StepDriver();;
    }
    else
    {
        setLastError("Simulation::ParseDriver Type=\""s + buf + "\" not recognised"s);
        return lastErrorPtr();
    }

    driver->setSimulation(this);
    driver->CreateAttributeMap(node->names, node->values);
    errorMessage = driver->CreateFromAttributes();
    if (errorMessage)
    {
        setLastError(*errorMessage);
        if (driver) delete driver;
        return lastErrorPtr();
    }

    m_DriverList[driver->GetName()] = driver;
    return nullptr;
}


void Simulation::ParseDataTarget(const ParseXML::XMLElement *node)
{
//    char *buf;
//    int count;
//    DataTarget *dataTarget;
//    buf = DoXmlGetProp(cur, "Type");
//    char defType[] = "Scalar";
//    if (buf == nullptr) buf = defType;

//    if (strcmp((const char *)buf, "Scalar") == 0)
//    {
//        DataTargetScalar *dataTargetScalar = new DataTargetScalar();
//        dataTargetScalar->setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        dataTargetScalar->SetName((const char *)buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "DataType"));
//        if (strcmp((const char *)buf, "XP") == 0) dataTargetScalar->SetDataType(DataTargetScalar::XP);
//        else if (strcmp((const char *)buf, "YP") == 0) dataTargetScalar->SetDataType(DataTargetScalar::YP);
//        else if (strcmp((const char *)buf, "ZP") == 0) dataTargetScalar->SetDataType(DataTargetScalar::ZP);
//        else if (strcmp((const char *)buf, "Q0") == 0) dataTargetScalar->SetDataType(DataTargetScalar::Q0);
//        else if (strcmp((const char *)buf, "Q1") == 0) dataTargetScalar->SetDataType(DataTargetScalar::Q1);
//        else if (strcmp((const char *)buf, "Q2") == 0) dataTargetScalar->SetDataType(DataTargetScalar::Q2);
//        else if (strcmp((const char *)buf, "Q3") == 0) dataTargetScalar->SetDataType(DataTargetScalar::Q3);
//        else if (strcmp((const char *)buf, "XV") == 0) dataTargetScalar->SetDataType(DataTargetScalar::XV);
//        else if (strcmp((const char *)buf, "YV") == 0) dataTargetScalar->SetDataType(DataTargetScalar::YV);
//        else if (strcmp((const char *)buf, "ZV") == 0) dataTargetScalar->SetDataType(DataTargetScalar::ZV);
//        else if (strcmp((const char *)buf, "XRV") == 0) dataTargetScalar->SetDataType(DataTargetScalar::XRV);
//        else if (strcmp((const char *)buf, "YRV") == 0) dataTargetScalar->SetDataType(DataTargetScalar::YRV);
//        else if (strcmp((const char *)buf, "ZRV") == 0) dataTargetScalar->SetDataType(DataTargetScalar::ZRV);
//        else if (strcmp((const char *)buf, "Angle") == 0) dataTargetScalar->SetDataType(DataTargetScalar::Angle);
//        else if (strcmp((const char *)buf, "MetabolicEnergy") == 0) dataTargetScalar->SetDataType(DataTargetScalar::MetabolicEnergy);
//        else if (strcmp((const char *)buf, "MechanicalEnergy") == 0) dataTargetScalar->SetDataType(DataTargetScalar::MechanicalEnergy);
//        else if (strcmp((const char *)buf, "DriverError") == 0) dataTargetScalar->SetDataType(DataTargetScalar::DriverError);
//        else throw(__LINE__);

//        if (dataTargetScalar->GetDataType() != DataTargetScalar::MetabolicEnergy && dataTargetScalar->GetDataType() != DataTargetScalar::MechanicalEnergy)
//        {
//            THROWIFZERO(buf = DoXmlGetProp(cur, "TargetID"));
//            std::map<std::string, Body *>::const_iterator iterBody = m_BodyList.find(buf);
//            if (iterBody != m_BodyList.end()) dataTargetScalar->SetTarget(iterBody->second);
//            std::map<std::string, Joint *>::const_iterator iterJoint = m_JointList.find(buf);
//            if (iterJoint != m_JointList.end()) dataTargetScalar->SetTarget(iterJoint->second);
//            std::map<std::string, Geom *>::const_iterator iterGeom = m_GeomList.find(buf);
//            if (iterGeom != m_GeomList.end()) dataTargetScalar->SetTarget(iterGeom->second);
//            std::map<std::string, Reporter *>::const_iterator iterReporter = m_ReporterList.find(buf);
//            if (iterReporter != m_ReporterList.end()) dataTargetScalar->SetTarget(iterReporter->second);
//            std::map<std::string, Driver *>::const_iterator iterDriver = m_DriverList.find(buf);
//            if (iterDriver != m_DriverList.end()) dataTargetScalar->SetTarget(iterDriver->second);
//            THROWIFZERO(dataTargetScalar->GetTarget());
//        }

//        THROWIFZERO(buf = DoXmlGetProp(cur, "Intercept"));
//        dataTargetScalar->SetIntercept(GSUtil::Double(buf));
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Slope"));
//        dataTargetScalar->SetSlope(GSUtil::Double(buf));
//        THROWIFZERO(buf = DoXmlGetProp(cur, "MatchType"));
//        if (strcmp((const char *)buf, "Linear") == 0) dataTargetScalar->SetMatchType(DataTarget::linear);
//        else if (strcmp((const char *)buf, "Square") == 0) dataTargetScalar->SetMatchType(DataTarget::square);
//        else throw(__LINE__);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetTimes"));
//        count = DataFile::CountTokens((char *)buf);
//        GSUtil::Double(buf, count, m_DoubleList);
//        dataTargetScalar->SetTargetTimes(count, m_DoubleList);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetValues"));
//        count = DataFile::CountTokens((char *)buf);
//        GSUtil::Double(buf, count, m_DoubleList);
//        dataTargetScalar->SetTargetValues(count, m_DoubleList);

//        // check presence of AbortThreshold
//        buf = DoXmlGetProp(cur, "AbortThreshold");
//        if (buf)
//        {
//            dataTargetScalar->SetAbortThreshold(GSUtil::Double(buf));
//        }

//        dataTarget = dataTargetScalar;
//    }
//    else if (strcmp((const char *)buf, "Quaternion") == 0)
//    {
//        DataTargetQuaternion *dataTargetQuaternion = new DataTargetQuaternion();
//        dataTargetQuaternion->setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        dataTargetQuaternion->SetName((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetID"));
//        std::map<std::string, Body *>::const_iterator iterBody = m_BodyList.find(buf);
//        if (iterBody != m_BodyList.end()) dataTargetQuaternion->SetTarget(iterBody->second);
//        std::map<std::string, Geom *>::const_iterator iterGeom = m_GeomList.find(buf);
//        if (iterGeom != m_GeomList.end()) dataTargetQuaternion->SetTarget(iterGeom->second);
//        std::map<std::string, Reporter *>::const_iterator iterReporter = m_ReporterList.find(buf);
//        if (iterReporter != m_ReporterList.end()) dataTargetQuaternion->SetTarget(iterReporter->second);
//        THROWIFZERO(dataTargetQuaternion->GetTarget());

//        THROWIFZERO(buf = DoXmlGetProp(cur, "Intercept"));
//        dataTargetQuaternion->SetIntercept(GSUtil::Double(buf));
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Slope"));
//        dataTargetQuaternion->SetSlope(GSUtil::Double(buf));
//        THROWIFZERO(buf = DoXmlGetProp(cur, "MatchType"));
//        if (strcmp((const char *)buf, "Linear") == 0) dataTargetQuaternion->SetMatchType(DataTarget::linear);
//        else if (strcmp((const char *)buf, "Square") == 0) dataTargetQuaternion->SetMatchType(DataTarget::square);
//        else throw(__LINE__);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetTimes"));
//        count = DataFile::CountTokens((char *)buf);
//        GSUtil::Double(buf, count, m_DoubleList);
//        dataTargetQuaternion->SetTargetTimes(count, m_DoubleList);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetValues"));
//        count = DataFile::CountTokens((char *)buf);
//        GSUtil::Double(buf, count, m_DoubleList);
//        dataTargetQuaternion->SetTargetValues(count, m_DoubleList);

//        // check presence of AbortThreshold
//        buf = DoXmlGetProp(cur, "AbortThreshold");
//        if (buf)
//        {
//            dataTargetQuaternion->SetAbortThreshold(GSUtil::Double(buf));
//        }

//        dataTarget = dataTargetQuaternion;
//    }
//    else if (strcmp((const char *)buf, "Vector") == 0)
//    {
//        DataTargetVector *dataTargetVector = new DataTargetVector();
//        dataTargetVector->setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        dataTargetVector->SetName((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetID"));
//        std::map<std::string, Body *>::const_iterator iterBody = m_BodyList.find(buf);
//        if (iterBody != m_BodyList.end()) dataTargetVector->SetTarget(iterBody->second);
//        std::map<std::string, Joint *>::const_iterator iterJoint = m_JointList.find(buf);
//        if (iterJoint != m_JointList.end()) dataTargetVector->SetTarget(iterJoint->second);
//        std::map<std::string, Geom *>::const_iterator iterGeom = m_GeomList.find(buf);
//        if (iterGeom != m_GeomList.end()) dataTargetVector->SetTarget(iterGeom->second);
//        std::map<std::string, Reporter *>::const_iterator iterReporter = m_ReporterList.find(buf);
//        if (iterReporter != m_ReporterList.end()) dataTargetVector->SetTarget(iterReporter->second);
//        THROWIFZERO(dataTargetVector->GetTarget());

//        THROWIFZERO(buf = DoXmlGetProp(cur, "Intercept"));
//        dataTargetVector->SetIntercept(GSUtil::Double(buf));
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Slope"));
//        dataTargetVector->SetSlope(GSUtil::Double(buf));
//        THROWIFZERO(buf = DoXmlGetProp(cur, "MatchType"));
//        if (strcmp((const char *)buf, "Linear") == 0) dataTargetVector->SetMatchType(DataTarget::linear);
//        else if (strcmp((const char *)buf, "Square") == 0) dataTargetVector->SetMatchType(DataTarget::square);
//        else throw(__LINE__);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetTimes"));
//        count = DataFile::CountTokens((char *)buf);
//        GSUtil::Double(buf, count, m_DoubleList);
//        dataTargetVector->SetTargetTimes(count, m_DoubleList);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetValues"));
//        count = DataFile::CountTokens((char *)buf);
//        GSUtil::Double(buf, count, m_DoubleList);
//        dataTargetVector->SetTargetValues(count, m_DoubleList);

//        // check presence of AbortThreshold
//        buf = DoXmlGetProp(cur, "AbortThreshold");
//        if (buf)
//        {
//            dataTargetVector->SetAbortThreshold(GSUtil::Double(buf));
//        }

//        dataTarget = dataTargetVector;
//    }

//    else
//    {
//        throw __LINE__;
//    }

//    m_DataTargetList[dataTarget->GetName()] = dataTarget;

}

void Simulation::ParseReporter(const ParseXML::XMLElement *node)
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

//    m_ReporterList[reporter->GetName()] = reporter;
}

void Simulation::ParseController(const ParseXML::XMLElement *node)
{
//    char *buf;
//    int count;
//    THROWIFZERO(buf = DoXmlGetProp(cur, "Type"));

//    if (strcmp((const char *)buf, "PIDMuscleLength") == 0)
//    {
//        PIDMuscleLength *pidMuscleLength = new PIDMuscleLength();
//        pidMuscleLength->Driver::setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        m_ControllerList[(const char *)buf] = pidMuscleLength;
//        pidMuscleLength->Driver::SetName((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "MuscleID"));
//        pidMuscleLength->SetMuscle(m_MuscleList[(const char *)buf]);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "NominalLength"));
//        pidMuscleLength->SetNominalLength(GSUtil::Double(buf));

//        THROWIFZERO(buf = DoXmlGetProp(cur, "Kp"));
//        double Kp = GSUtil::Double(buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Ki"));
//        double Ki = GSUtil::Double(buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Kd"));
//        double Kd = GSUtil::Double(buf);
//        pidMuscleLength->SetPID(Kp, Ki, Kd);

//        buf = DoXmlGetProp(cur, "DriverRange");
//        if (buf)
//        {
//            count = DataFile::CountTokens((char *)buf);
//            if (count >= 2)
//            {
//                GSUtil::Double(buf, count, m_DoubleList);
//                pidMuscleLength->SetMinMax(m_DoubleList[0],m_DoubleList[1]);
//            }
//        }

//    }

//    else if (strcmp((const char *)buf, "PIDTargetMatch") == 0)
//    {
//        PIDTargetMatch *pidTargetMatch = new PIDTargetMatch();
//        pidTargetMatch->Driver::setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        m_ControllerList[(const char *)buf] = pidTargetMatch;
//        pidTargetMatch->Driver::SetName((const char *)buf);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "MuscleID"));
//        pidTargetMatch->SetMuscle(m_MuscleList[(const char *)buf]);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "DataTargetID"));
//        pidTargetMatch->SetDataTarget(m_DataTargetList[(const char *)buf]);

//        THROWIFZERO(buf = DoXmlGetProp(cur, "Kp"));
//        double Kp = GSUtil::Double(buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Ki"));
//        double Ki = GSUtil::Double(buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Kd"));
//        double Kd = GSUtil::Double(buf);
//        pidTargetMatch->SetPID(Kp, Ki, Kd);

//        buf = DoXmlGetProp(cur, "DriverRange");
//        if (buf)
//        {
//            count = DataFile::CountTokens((char *)buf);
//            if (count >= 2)
//            {
//                GSUtil::Double(buf, count, m_DoubleList);
//                pidTargetMatch->SetMinMax(m_DoubleList[0],m_DoubleList[1]);
//            }
//        }

//    }

//    else if (strcmp((const char *)buf, "PIDErrorIn") == 0)
//    {
//        PIDErrorInController *pidErrorInController = new PIDErrorInController();
//        pidErrorInController->Driver::setSimulation(this);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "ID"));
//        m_ControllerList[(const char *)buf] = pidErrorInController;
//        pidErrorInController->Driver::SetName((const char *)buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "TargetID"));
//        if (m_MuscleList.find((const char *)buf) != m_MuscleList.end()) pidErrorInController->SetTarget(m_MuscleList[(const char *)buf]);
//        else if (m_ControllerList.find((const char *)buf) != m_ControllerList.end()) pidErrorInController->SetTarget(m_ControllerList[(const char *)buf]);
//        else throw __LINE__;
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Kp"));
//        double Kp = GSUtil::Double(buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Ki"));
//        double Ki = GSUtil::Double(buf);
//        THROWIFZERO(buf = DoXmlGetProp(cur, "Kd"));
//        double Kd = GSUtil::Double(buf);
//        pidErrorInController->Initialise(Kp, Ki, Kd);
////        buf = DoXmlGetProp(cur, "OutputIsDelta");
////        if (buf) pidErrorInController->setOutputIsDelta(GSUtil::Bool(buf));

//        buf = DoXmlGetProp(cur, "DriverRange");
//        if (buf)
//        {
//            count = DataFile::CountTokens((char *)buf);
//            if (count >= 2)
//            {
//                GSUtil::Double(buf, count, m_DoubleList);
//                pidErrorInController->SetMinMax(m_DoubleList[0],m_DoubleList[1]);
//            }
//        }
//    }

//    else
//    {
//        throw __LINE__;
//    }
}

void Simulation::ParseWarehouse(const ParseXML::XMLElement *node)
{
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
            for (std::map<std::string, Driver *>::const_iterator iter = m_DriverList.begin(); iter != m_DriverList.end(); iter++) m_OutputWarehouseFile << "\t\"" << iter->second->GetName() << "\"";
            m_OutputWarehouseFile << "\t" << m_BodyList.size();
            m_OutputWarehouseFile << "\t" << m_BodyList[m_global.DistanceTravelledBodyIDName()]->GetName();
            for (std::map<std::string, Body *>::const_iterator iter = m_BodyList.begin(); iter != m_BodyList.end(); iter++)
                if (iter->second->GetName() != m_global.DistanceTravelledBodyIDName()) m_OutputWarehouseFile << "\t\"" << iter->second->GetName() << "\"";
            m_OutputWarehouseFile << "\n";
        }

        m_OutputWarehouseLastTime = m_SimulationTime;
        // simulation time
        m_OutputWarehouseFile << m_SimulationTime;
        // driver activations
        for (std::map<std::string, Driver *>::const_iterator iter = m_DriverList.begin(); iter != m_DriverList.end(); iter++) m_OutputWarehouseFile << "\t" << iter->second->GetValue(m_SimulationTime);
        // output the root body (m_global.DistanceTravelledBodyIDName())
        Body *rootBody = m_BodyList[m_global.DistanceTravelledBodyIDName()];
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
        for (std::map<std::string, Body *>::const_iterator iter = m_BodyList.begin(); iter != m_BodyList.end(); iter++)
        {
            if (iter->second->GetName() != m_global.DistanceTravelledBodyIDName())
            {
                iter->second->GetRelativePosition(rootBody, &pos);
                iter->second->GetRelativeQuaternion(rootBody, &quat);
                iter->second->GetRelativeLinearVelocity(rootBody, &vel);
                iter->second->GetRelativeAngularVelocity(rootBody, &avel);
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
            for (std::map<std::string, Driver *>::const_iterator iter = m_DriverList.begin(); iter != m_DriverList.end(); iter++) GSUtil::BinaryOutput(m_OutputWarehouseFile, iter->second->GetName());
            GSUtil::BinaryOutput(m_OutputWarehouseFile, uint32_t(m_BodyList.size()));
            GSUtil::BinaryOutput(m_OutputWarehouseFile, m_BodyList[m_global.DistanceTravelledBodyIDName()]->GetName());
            for (std::map<std::string, Body *>::const_iterator iter = m_BodyList.begin(); iter != m_BodyList.end(); iter++)
                if (iter->second->GetName() != m_global.DistanceTravelledBodyIDName()) GSUtil::BinaryOutput(m_OutputWarehouseFile, iter->second->GetName());
        }

        m_OutputWarehouseLastTime = m_SimulationTime;
        // simulation time
        GSUtil::BinaryOutput(m_OutputWarehouseFile, m_SimulationTime);
        // driver activations
        for (std::map<std::string, Driver *>::const_iterator iter = m_DriverList.begin(); iter != m_DriverList.end(); iter++) GSUtil::BinaryOutput(m_OutputWarehouseFile, iter->second->GetValue(m_SimulationTime));
        // output the root body (m_global.DistanceTravelledBodyIDName())
        Body *rootBody = m_BodyList[m_global.DistanceTravelledBodyIDName()];
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
        for (std::map<std::string, Body *>::const_iterator iter = m_BodyList.begin(); iter != m_BodyList.end(); iter++)
        {
            if (iter->second->GetName() != m_global.DistanceTravelledBodyIDName())
            {
                iter->second->GetRelativePosition(rootBody, &pos);
                iter->second->GetRelativeQuaternion(rootBody, &quat);
                iter->second->GetRelativeLinearVelocity(rootBody, &vel);
                iter->second->GetRelativeAngularVelocity(rootBody, &avel);
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
    auto elementList = m_parseXML.elementList();
    elementList->clear();

    m_global.SaveToAttributes(); m_parseXML.AddElement("GLOBAL"s, m_global.getAttributeMap());
    for (auto it : m_BodyList) { it.second->SaveToAttributes(); m_parseXML.AddElement("BODY"s, it.second->getAttributeMap()); }
    for (auto it : m_MarkerList) { it.second->SaveToAttributes(); m_parseXML.AddElement("MARKER"s, it.second->getAttributeMap()); }
    for (auto it : m_JointList) { it.second->SaveToAttributes(); m_parseXML.AddElement("JOINT"s, it.second->getAttributeMap()); }
    for (auto it : m_GeomList) { it.second->SaveToAttributes(); m_parseXML.AddElement("GEOM"s, it.second->getAttributeMap()); }
    for (auto it : m_StrapList) { it.second->SaveToAttributes(); m_parseXML.AddElement("STRAP"s, it.second->getAttributeMap()); }
    for (auto it : m_MuscleList) { it.second->SaveToAttributes(); m_parseXML.AddElement("MUSCLE"s, it.second->getAttributeMap()); }
    for (auto it : m_FluidSacList) { it.second->SaveToAttributes(); m_parseXML.AddElement("FLUIDSAC"s, it.second->getAttributeMap()); }
    for (auto it : m_ReporterList) { it.second->SaveToAttributes(); m_parseXML.AddElement("REPORTER"s, it.second->getAttributeMap()); }
//    for (auto it : m_ControllerList) { it.second->SaveToAttributes(); m_parseXML.AddElement("CONTROLLER"s, it.second->getAttributeMap()); }
    for (auto it : m_WarehouseList) { it.second->SaveToAttributes(); m_parseXML.AddElement("WAREHOUSE"s, it.second->getAttributeMap()); }
    for (auto it : m_DataTargetList) { it.second->SaveToAttributes(); m_parseXML.AddElement("DATATARGET"s, it.second->getAttributeMap()); }
    for (auto it : m_DriverList) { it.second->SaveToAttributes(); m_parseXML.AddElement("DRIVER"s, it.second->getAttributeMap()); }

    // convert to string and output
    std::string xmlString = m_parseXML.SaveModel();
    DataFile outputFile;
    outputFile.SetRawData(xmlString.c_str(), xmlString.size());
    outputFile.WriteFile(m_OutputModelStateFile);

    // element list now contains a collection of pointers to unreliable data so clear it
    elementList->clear();
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
    m_global.setWarehouseFailDistanceAbort(warehouseFailDistanceAbort);
    if (m_global.fitnessType() == Global::ClosestWarehouse) m_global.setFitnessType(Global::DistanceTravelled);
}

void Simulation::SetGlobal(const Global &global)
{
    m_global = global;
    // set the global simulation parameters
    dWorldSetGravity(m_WorldID, m_global.Gravity().x, m_global.Gravity().y, m_global.Gravity().z);
    dWorldSetERP(m_WorldID, m_global.ERP());
    dWorldSetCFM(m_WorldID, m_global.CFM());
    dWorldSetContactMaxCorrectingVel(m_WorldID, m_global.ContactMaxCorrectingVel());
    dWorldSetContactSurfaceLayer(m_WorldID, m_global.ContactSurfaceLayer());
}

// add a warehouse from a file
void Simulation::AddWarehouse(const char *filename)
{
}

bool Simulation::ShouldQuit()
{
    if (m_global.TimeLimit() > 0)
        if (m_SimulationTime > m_global.TimeLimit()) return true;
    if (m_global.MechanicalEnergyLimit() > 0)
        if (m_MechanicalEnergy > m_global.MechanicalEnergyLimit()) return true;
    if (m_global.MetabolicEnergyLimit() > 0)
        if (m_MetabolicEnergy > m_global.MetabolicEnergyLimit()) return true;
    return false;
}

// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

void Simulation::NearCallback(void *data, dGeomID o1, dGeomID o2)
{
    int i;
    int numc;
    Simulation *s = reinterpret_cast<Simulation *>(data);

    // exit without doing anything if the two bodies are connected by a joint
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);
    Contact *myContact;

    if (s->m_global.AllowConnectedCollisions() == false)
    {
        if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact)) return;
    }

    if (s->m_global.AllowInternalCollisions() == false)
    {
        if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetGeomLocation() == reinterpret_cast<Geom *>(dGeomGetData(o2))->GetGeomLocation()) return;
    }

    dContact *contact = new dContact[size_t(s->m_MaxContacts)];   // up to m_MaxContacts contacts per box-box
    double cfm = MAX(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactSoftCFM(),
                     reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactSoftCFM());
    double erp = MIN(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactSoftERP(),
                     reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactSoftERP());
    double mu = MIN(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactMu(),
                    reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactMu());
    double bounce = MAX(reinterpret_cast<Geom *>(dGeomGetData(o1))->GetContactBounce(),
                        reinterpret_cast<Geom *>(dGeomGetData(o2))->GetContactBounce());
    for (i = 0; i < s->m_MaxContacts; i++)
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
        for (i = 0; i < numc; i++)
        {
            if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetAbort()) s->SetContactAbort(true);
            if (reinterpret_cast<Geom *>(dGeomGetData(o2))->GetAbort()) s->SetContactAbort(true);
            dJointID c;
            if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetAdhesion() == false && reinterpret_cast<Geom *>(dGeomGetData(o2))->GetAdhesion() == false)
            {
                c = dJointCreateContact(s->m_WorldID, s->m_ContactGroup, contact + i);
                dJointAttach(c, b1, b2);
                myContact = new Contact();
                myContact->setSimulation(s);
                dJointSetFeedback(c, myContact->GetJointFeedback());
                myContact->SetJointID(c);
                std::copy_n(contact[i].geom.pos, dV3E__MAX, myContact->GetContactPosition());
                s->m_ContactList.push_back(myContact);
                // only add the contact information once
                // and add it to the non-environment geom
                if (reinterpret_cast<Geom *>(dGeomGetData(o1))->GetGeomLocation() == Geom::environment)
                    reinterpret_cast<Geom *>(dGeomGetData(o2))->AddContact(myContact);
                else
                    reinterpret_cast<Geom *>(dGeomGetData(o1))->AddContact(myContact);
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
    delete [] contact;
}

Body *Simulation::GetBody(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_BodyList.find(name);
    if (iter != m_BodyList.end()) return iter->second;
    return nullptr;
}

Joint *Simulation::GetJoint(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_JointList.find(name);
    if (iter != m_JointList.end()) return iter->second;
    return nullptr;
}

Geom *Simulation::GetGeom(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_GeomList.find(name);
    if (iter != m_GeomList.end()) return iter->second;
    return nullptr;
}

Muscle *Simulation::GetMuscle(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_MuscleList.find(name);
    if (iter != m_MuscleList.end()) return iter->second;
    return nullptr;
}

Strap *Simulation::GetStrap(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_StrapList.find(name);
    if (iter != m_StrapList.end()) return iter->second;
    return nullptr;
}

FluidSac *Simulation::GetFluidSac(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_FluidSacList.find(name);
    if (iter != m_FluidSacList.end()) return iter->second;
    return nullptr;
}

Driver *Simulation::GetDriver(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_DriverList.find(name);
    if (iter != m_DriverList.end()) return iter->second;
    return nullptr;
}

DataTarget *Simulation::GetDataTarget(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_DataTargetList.find(name);
    if (iter != m_DataTargetList.end()) return iter->second;
    return nullptr;
}

Marker *Simulation::GetMarker(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_MarkerList.find(name);
    if (iter != m_MarkerList.end()) return iter->second;
    return nullptr;
}

Reporter *Simulation::GetReporter(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_ReporterList.find(name);
    if (iter != m_ReporterList.end()) return iter->second;
    return nullptr;
}

Controller *Simulation::GetController(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_ControllerList.find(name);
    if (iter != m_ControllerList.end()) return iter->second;
    return nullptr;
}

Warehouse *Simulation::GetWarehouse(const std::string &name)
{
    // use find to allow null return if name not found
    auto iter = m_WarehouseList.find(name);
    if (iter != m_WarehouseList.end()) return iter->second;
    return nullptr;
}

const Global &Simulation::GetGlobal() const
{
    return m_global;
}

// this version of the dump routine simply calls the dump functions of the embedded objects
void Simulation::Dump()
{
    std::map<std::string, Body *>::const_iterator BodyIter;
    for (BodyIter = m_BodyList.begin(); BodyIter != m_BodyList.end(); BodyIter++) BodyIter->second->Dump();

    std::map<std::string, Marker *>::const_iterator MarkerIter;
    for (MarkerIter = m_MarkerList.begin(); MarkerIter != m_MarkerList.end(); MarkerIter++) MarkerIter->second->Dump();

    std::map<std::string, Joint *>::const_iterator JointIter;
    for (JointIter = m_JointList.begin(); JointIter != m_JointList.end(); JointIter++) JointIter->second->Dump();

    std::map<std::string, Geom *>::const_iterator GeomIter;
    for (GeomIter = m_GeomList.begin(); GeomIter != m_GeomList.end(); GeomIter++) GeomIter->second->Dump();

    std::map<std::string, Muscle *>::const_iterator MuscleIter;
    for (MuscleIter = m_MuscleList.begin(); MuscleIter != m_MuscleList.end(); MuscleIter++) { MuscleIter->second->Dump(); MuscleIter->second->GetStrap()->Dump(); }

    std::map<std::string, FluidSac *>::const_iterator FluidSacIter;
    for (FluidSacIter = m_FluidSacList.begin(); FluidSacIter != m_FluidSacList.end(); FluidSacIter++) FluidSacIter->second->Dump();

    std::map<std::string, Driver *>::const_iterator DriverIter;
    for (DriverIter = m_DriverList.begin(); DriverIter != m_DriverList.end(); DriverIter++) DriverIter->second->Dump();

    std::map<std::string, DataTarget *>::const_iterator DataTargetIter;
    for (DataTargetIter = m_DataTargetList.begin(); DataTargetIter != m_DataTargetList.end(); DataTargetIter++) DataTargetIter->second->Dump();

    std::map<std::string, Reporter *>::const_iterator ReporterIter;
    for (ReporterIter = m_ReporterList.begin(); ReporterIter != m_ReporterList.end(); ReporterIter++) ReporterIter->second->Dump();

    std::map<std::string, Warehouse *>::const_iterator WarehouseIter;
    for (WarehouseIter = m_WarehouseList.begin(); WarehouseIter != m_WarehouseList.end(); WarehouseIter++) WarehouseIter->second->Dump();

}



