/*
 *  Simulation.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// Simulation.h - this simulation object is used to encapsulate a simulation

#ifndef SIMULATION_H
#define SIMULATION_H

#include "DataFile.h"
#include "Global.h"
#include "Contact.h"
#include "ParseXML.h"

#include "ode/ode.h"

#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <set>

class Body;
class Joint;
class Geom;
class Muscle;
class Strap;
class FluidSac;
class Driver;
class DataTarget;
class Contact;
class Marker;
class Reporter;
class Controller;
class FixedJoint;
class Warehouse;
class SimulationWindow;
class MainWindow;

class Simulation : NamedObject
{
public:

    Simulation();
    virtual ~Simulation();

    enum AxisType
    {
        XAxis,
        YAxis,
        ZAxis
    };

    static void NearCallback(void *data, dGeomID o1, dGeomID o2);

    std::string *LoadModel(const char *buffer, size_t length);  // load parameters from the XML configuration file
    void UpdateSimulation(void);     // called at each iteration through simulation

    // get hold of various variables

    double GetTime(void) { return m_SimulationTime; }
    double GetTimeIncrement(void) { return m_global.StepSize(); }
    long long GetStepCount(void) { return m_StepCount; }
    double GetMechanicalEnergy(void) { return m_MechanicalEnergy; }
    double GetMetabolicEnergy(void) { return m_MetabolicEnergy; }
    double GetTimeLimit(void) { return m_global.TimeLimit(); }
    double GetMetabolicEnergyLimit(void) { return m_global.MetabolicEnergyLimit(); }
    double GetMechanicalEnergyLimit(void) { return m_global.MechanicalEnergyLimit(); }
    Body *GetBody(const std::string &name);
    Joint *GetJoint(const std::string &name);
    Geom *GetGeom(const std::string &name);
    Muscle *GetMuscle(const std::string &name);
    Strap *GetStrap(const std::string &name);
    FluidSac *GetFluidSac(const std::string &name);
    Driver *GetDriver(const std::string &name);
    DataTarget *GetDataTarget(const std::string &name);
    Marker *GetMarker(const std::string &name);
    Reporter *GetReporter(const std::string &name);
    Controller *GetController(const std::string &name);
    Warehouse *GetWarehouse(const std::string &name);
    bool GetOutputModelStateOccured() { return m_OutputModelStateOccured; }
    dWorldID GetWorldID() { return m_WorldID; }
    dSpaceID GetSpaceID() { return m_SpaceID; }

    void SetTimeLimit(double timeLimit) { m_global.setTimeLimit(timeLimit); }
    void SetMetabolicEnergyLimit(double energyLimit) { m_global.setMetabolicEnergyLimit(energyLimit); }
    void SetMechanicalEnergyLimit(double energyLimit) { m_global.setMechanicalEnergyLimit(energyLimit); }
    void SetOutputModelStateAtTime(double outputModelStateAtTime) { m_OutputModelStateAtTime = outputModelStateAtTime; }
    void SetOutputModelStateAtCycle(double outputModelStateAtCycle) { m_OutputModelStateAtCycle = outputModelStateAtCycle; }
    void SetOutputModelStateAtWarehouseDistance(double outputModelStateAtWarehouseDistance) { m_OutputModelStateAtWarehouseDistance = outputModelStateAtWarehouseDistance; }
    void SetOutputModelStateFile(const std::string &filename);
    void SetOutputWarehouseFile(const std::string &filename);
    void SetMungeModelStateFlag(bool f) { m_MungeModelStateFlag = f; }
    void SetMungeRotationFlag(bool f) { m_MungeRotationFlag = f; }
    void SetModelStateRelative(bool f) { m_ModelStateRelative = f; }
    void SetWarehouseFailDistanceAbort(double warehouseFailDistanceAbort);

    void AddWarehouse(const char *filename);

    // get hold of the internal lists (HANDLE WITH CARE)
    std::map<std::string, Body *> *GetBodyList() { return &m_BodyList; }
    std::map<std::string, Joint *> *GetJointList() { return &m_JointList; }
    std::map<std::string, Geom *> *GetGeomList() { return &m_GeomList; }
    std::map<std::string, Muscle *> *GetMuscleList() { return &m_MuscleList; }
    std::map<std::string, Strap *> *GetStrapList() { return &m_StrapList; }
    std::map<std::string, FluidSac *> *GetFluidSacList() { return &m_FluidSacList; }
    std::map<std::string, Driver *> *GetDriverList() { return &m_DriverList; }
    std::map<std::string, DataTarget *> *GetDataTargetList() { return &m_DataTargetList; }
    std::map<std::string, Marker *> *GetMarkerList() { return &m_MarkerList; }
    std::map<std::string, Reporter *> *GetReporterList() { return &m_ReporterList; }
    std::map<std::string, Controller *> *GetControllerList() { return &m_ControllerList; }
    std::map<std::string, Warehouse *> *GetWarehouseList() { return &m_WarehouseList; }
    std::vector<Contact *> *GetContactList() { return &m_ContactList; }
    std::map<std::string, NamedObject *> *GetObjectList() { return &m_objectList; }
    std::map<std::string, std::set<std::string>> *GetGroupList() { return &m_groupList; }

    // fitness related values
    bool TestForCatastrophy();
    double CalculateInstantaneousFitness();
    bool ShouldQuit();
    void SetContactAbort(bool contactAbort) { m_ContactAbort = contactAbort; }
    void SetDataTargetAbort(bool dataTargetAbort) { m_DataTargetAbort = dataTargetAbort; }

    // these should probably only be internal
    void OutputProgramState();
    void OutputWarehouse();

    virtual void Dump();

    void SetMainWindow(MainWindow *mainWindow) { m_MainWindow = mainWindow; }
    MainWindow *GetMainWindow() { return m_MainWindow; }

    const Global &GetGlobal() const;
    void SetGlobal(const Global &global);

private:

    std::string *ParseGlobal(const ParseXML::XMLElement *node);
    std::string *ParseBody(const ParseXML::XMLElement *node);
    std::string *ParseGeom(const ParseXML::XMLElement *node);
    std::string *ParseJoint(const ParseXML::XMLElement *node);
    std::string *ParseMuscle(const ParseXML::XMLElement *node);
    std::string *ParseStrap(const ParseXML::XMLElement *node);
    std::string *ParseMarker(const ParseXML::XMLElement *node);
    std::string *ParseFluidSac(const ParseXML::XMLElement *node);
    std::string *ParseDriver(const ParseXML::XMLElement *node);
    void ParseDataTarget(const ParseXML::XMLElement *node);
    void ParseReporter(const ParseXML::XMLElement *node);
    void ParseController(const ParseXML::XMLElement *node);
    void ParseWarehouse(const ParseXML::XMLElement *node);

    ParseXML m_parseXML;

   // these are the internal lists that are all owners of their respective objects
    std::map<std::string, Body *> m_BodyList;
    std::map<std::string, Joint *> m_JointList;
    std::map<std::string, Geom *> m_GeomList;
    std::map<std::string, Muscle *> m_MuscleList;
    std::map<std::string, Strap *> m_StrapList;
    std::map<std::string, FluidSac *> m_FluidSacList;
    std::map<std::string, Driver *> m_DriverList;
    std::map<std::string, DataTarget *> m_DataTargetList;
    std::map<std::string, Marker *> m_MarkerList;
    std::map<std::string, Reporter *> m_ReporterList;
    std::map<std::string, Controller *> m_ControllerList;
    std::map<std::string, Warehouse *> m_WarehouseList;

    // this is a list of contacts that are active at the current time step
    std::vector<Contact *> m_ContactList;

    // these are lists to access all objects by name
    std::map<std::string, NamedObject *> m_objectList;
    std::map<std::string, std::set<std::string>> m_groupList;

    // Simulation variables
    dWorldID m_WorldID;
    dSpaceID m_SpaceID;
    dJointGroupID m_ContactGroup;
    int m_MaxContacts = 64;
    Global m_global;

    // keep track of simulation time

    double m_SimulationTime = 0; // current time
    int64_t m_StepCount = 0; // number of steps taken
    double m_CycleTime = 0;

    // and calculated energy
    double m_MechanicalEnergy = 0;
    double m_MetabolicEnergy = 0;

    // FitnessType
    double m_KinematicMatchMiniMaxFitness = 0;
    double m_ClosestWarehouseFitness = -DBL_MAX;

    // some control values
    bool m_OutputWarehouseFlag = false;
    std::string m_OutputWarehouseFilename;
    std::ofstream m_OutputWarehouseFile;
    std::string m_OutputModelStateFile;
    bool m_OutputModelStateOccured = false;
    bool m_AbortAfterModelStateOutput = false;
    bool m_OutputWarehouseAsText = false;
    double m_OutputModelStateAtTime = -1;
    double m_OutputModelStateAtCycle = -1;
    bool m_MungeModelStateFlag = false;
    bool m_MungeRotationFlag = false;
    int m_SimulationError = false;
    bool m_ModelStateRelative = true;
    bool m_StraightenBody = false;
    bool m_AbortOnODEMessage = false;
    double m_WarehouseDistance = 0;
    bool m_OutputKinematicsFirstTimeFlag = false;
    double m_OutputWarehouseLastTime = -DBL_MAX;
    double m_OutputModelStateAtWarehouseDistance = 0;
    bool m_WarehouseUsePCA = true;
    bool m_DataTargetAbort = false;
    bool m_ContactAbort = false;

    // for fitness calculations
    double m_KinematicMatchFitness = 0;
    Body *m_DistanceTravelledBodyID = nullptr;

    // values for energy partition
    double m_PositiveMechanicalWork = 0;
    double m_NegativeMechanicalWork = 0;
    double m_PositiveContractileWork = 0;
    double m_NegativeContractileWork = 0;
    double m_PositiveSerialElasticWork = 0;
    double m_NegativeSerialElasticWork = 0;
    double m_PositiveParallelElasticWork = 0;
    double m_NegativeParallelElasticWork = 0;

    MainWindow *m_MainWindow = nullptr;

};



#endif //SIMULATION_H
