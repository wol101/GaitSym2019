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
#include "SmartEnum.h"
#include "ErrorHandler.h"

#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>

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
class Drivable;

class Simulation : NamedObject
{
public:

    Simulation();
    virtual ~Simulation();

    SMART_ENUM(AxisType, axisTypeStrings, axisTypeCount, XAxis, YAxis, ZAxis);
//    enum AxisType { XAxis, YAxis, ZAxis };

    static void NearCallback(void *data, dGeomID o1, dGeomID o2);

    std::string *LoadModel(const char *buffer, size_t length);  // load parameters from the XML configuration file
    void UpdateSimulation(void);     // called at each iteration through simulation

    // get hold of various variables

    double GetTime(void) { return m_SimulationTime; }
    double GetTimeIncrement(void) { return m_global->StepSize(); }
    long long GetStepCount(void) { return m_StepCount; }
    double GetMechanicalEnergy(void) { return m_MechanicalEnergy; }
    double GetMetabolicEnergy(void) { return m_MetabolicEnergy; }
    double GetTimeLimit(void) { return m_global->TimeLimit(); }
    double GetMetabolicEnergyLimit(void) { return m_global->MetabolicEnergyLimit(); }
    double GetMechanicalEnergyLimit(void) { return m_global->MechanicalEnergyLimit(); }
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

    void SetTimeLimit(double timeLimit) { m_global->setTimeLimit(timeLimit); }
    void SetMetabolicEnergyLimit(double energyLimit) { m_global->setMetabolicEnergyLimit(energyLimit); }
    void SetMechanicalEnergyLimit(double energyLimit) { m_global->setMechanicalEnergyLimit(energyLimit); }
    void SetOutputModelStateAtTime(double outputModelStateAtTime) { m_OutputModelStateAtTime = outputModelStateAtTime; }
    void SetOutputModelStateAtCycle(double outputModelStateAtCycle) { m_OutputModelStateAtCycle = outputModelStateAtCycle; }
    void SetOutputModelStateAtWarehouseDistance(double outputModelStateAtWarehouseDistance) { m_OutputModelStateAtWarehouseDistance = outputModelStateAtWarehouseDistance; }
    void SetOutputModelStateFile(const std::string &filename);
    void SetOutputWarehouseFile(const std::string &filename);
    void SetWarehouseFailDistanceAbort(double warehouseFailDistanceAbort);

    void AddWarehouse(const std::string &filename);

    // get hold of the internal lists (HANDLE WITH CARE)
    std::map<std::string, std::unique_ptr<Body>> *GetBodyList() { return &m_BodyList; }
    std::map<std::string, std::unique_ptr<Joint>> *GetJointList() { return &m_JointList; }
    std::map<std::string, std::unique_ptr<Geom>> *GetGeomList() { return &m_GeomList; }
    std::map<std::string, std::unique_ptr<Muscle>> *GetMuscleList() { return &m_MuscleList; }
    std::map<std::string, std::unique_ptr<Strap>> *GetStrapList() { return &m_StrapList; }
    std::map<std::string, std::unique_ptr<FluidSac>> *GetFluidSacList() { return &m_FluidSacList; }
    std::map<std::string, std::unique_ptr<Driver>> *GetDriverList() { return &m_DriverList; }
    std::map<std::string, std::unique_ptr<DataTarget>> *GetDataTargetList() { return &m_DataTargetList; }
    std::map<std::string, std::unique_ptr<Marker>> *GetMarkerList() { return &m_MarkerList; }
    std::map<std::string, std::unique_ptr<Reporter>> *GetReporterList() { return &m_ReporterList; }
    std::map<std::string, std::unique_ptr<Controller>> *GetControllerList() { return &m_ControllerList; }
    std::map<std::string, std::unique_ptr<Warehouse>> *GetWarehouseList() { return &m_WarehouseList; }
    std::vector<std::unique_ptr<Contact>> *GetContactList() { return &m_ContactList; }

    std::vector<std::string> GetNameList() const;
    std::set<std::string> GetNameSet() const;
    std::vector<NamedObject *> GetObjectList() const;
    NamedObject *GetNamedObject(const std::string &name) const;
    bool DeleteNamedObject(const std::string &name);

    bool HasAssembly();

    // fitness related values
    bool TestForCatastrophy();
    double CalculateInstantaneousFitness();
    bool ShouldQuit();
    void SetContactAbort(const std::string &contactID) { m_ContactAbort = true;  m_ContactAbortList.push_back(contactID); }
    void SetDataTargetAbort(const std::string &dataTargetID) { m_DataTargetAbort = true; m_DataTargetAbortList.push_back(dataTargetID); }
    int m_numericalErrorCount = 0;

    std::string SaveToXML();
    void OutputProgramState();
    void OutputWarehouse();

    Global *GetGlobal();
    void SetGlobal(std::unique_ptr<Global> global);

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
    std::string *ParseDataTarget(const ParseXML::XMLElement *node);
    std::string *ParseReporter(const ParseXML::XMLElement *node);
    std::string *ParseController(const ParseXML::XMLElement *node);
    std::string *ParseWarehouse(const ParseXML::XMLElement *node);

    void DumpObjects();
    void DumpObject(NamedObject *namedObject);

    ParseXML m_parseXML;

   // these are the internal lists that are all owners of their respective objects
    std::map<std::string, std::unique_ptr<Body>> m_BodyList;
    std::map<std::string, std::unique_ptr<Joint>> m_JointList;
    std::map<std::string, std::unique_ptr<Geom>> m_GeomList;
    std::map<std::string, std::unique_ptr<Muscle>> m_MuscleList;
    std::map<std::string, std::unique_ptr<Strap>> m_StrapList;
    std::map<std::string, std::unique_ptr<FluidSac>> m_FluidSacList;
    std::map<std::string, std::unique_ptr<Driver>> m_DriverList;
    std::map<std::string, std::unique_ptr<DataTarget>> m_DataTargetList;
    std::map<std::string, std::unique_ptr<Marker>> m_MarkerList;
    std::map<std::string, std::unique_ptr<Reporter>> m_ReporterList;
    std::map<std::string, std::unique_ptr<Controller>> m_ControllerList;
    std::map<std::string, std::unique_ptr<Warehouse>> m_WarehouseList;

    // this is a list of contacts that are active at the current time step
    std::vector<std::unique_ptr<Contact>> m_ContactList;

    // Simulation variables
    dWorldID m_WorldID;
    dSpaceID m_SpaceID;
    dJointGroupID m_ContactGroup;
    int m_MaxContacts = 64;
    std::unique_ptr<Global> m_global;

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
    int m_SimulationError = false;
    bool m_StraightenBody = false;
    double m_WarehouseDistance = 0;
    bool m_OutputKinematicsFirstTimeFlag = false;
    double m_OutputWarehouseLastTime = -DBL_MAX;
    double m_OutputModelStateAtWarehouseDistance = 0;
    bool m_WarehouseUsePCA = true;
    bool m_DataTargetAbort = false;
    bool m_ContactAbort = false;
    std::vector<std::string> m_DataTargetAbortList;
    std::vector<std::string> m_ContactAbortList;

    // for fitness calculations
    double m_KinematicMatchFitness = 0;

    // values for energy partition
    double m_PositiveMechanicalWork = 0;
    double m_NegativeMechanicalWork = 0;
    double m_PositiveContractileWork = 0;
    double m_NegativeContractileWork = 0;
    double m_PositiveSerialElasticWork = 0;
    double m_NegativeSerialElasticWork = 0;
    double m_PositiveParallelElasticWork = 0;
    double m_NegativeParallelElasticWork = 0;

    // values for dump output
    std::string m_dumpExtension = {".tab"};
    std::map<std::string, std::ofstream> m_dumpFileStreams;
    ErrorHandler m_errorHandler;

};



#endif //SIMULATION_H
