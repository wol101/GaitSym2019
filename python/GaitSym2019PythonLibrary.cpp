#include "GaitSym2019PythonLibrary.h"

#define MAX_ARGS 4096

#include "GSUtil.h"
#include "DataFile.h"
#include "Simulation.h"
#include "Reporter.h"
#include "DataTarget.h"
#include "Driver.h"
#include "Joint.h"
#include "Muscle.h"
#include "Body.h"
#include "Geom.h"
#include "ArgParse.h"

#include "pybind11/stl.h"
#include "pybind11/pybind11.h"

#include <iostream>

using namespace std::string_literals;

void initGaitsym2019(pybind11::module &m)
{
    pybind11::class_<GaitSym2019PythonLibrary>(m, "GaitSym2019")
        .def(pybind11::init<>())
        .def ("SetArguments", &GaitSym2019PythonLibrary::SetArguments)
        .def("Run", &GaitSym2019PythonLibrary::Run)
        .def("ReadModel", &GaitSym2019PythonLibrary::ReadModel)
        .def("SetXML", &GaitSym2019PythonLibrary::SetXML)
        .def("GetFitness", &GaitSym2019PythonLibrary::GetFitness);
}

PYBIND11_MODULE(GaitSym2019, m)
{
    // Optional docstring
    m.doc() = "GaitSym2019 python bindings";

    initGaitsym2019(m);
}

GaitSym2019PythonLibrary::GaitSym2019PythonLibrary()
{
    if (m_debug) std::cerr << "GaitSym2019PythonLibrary object constructed\n";
}

GaitSym2019PythonLibrary::~GaitSym2019PythonLibrary()
{
    if (m_debug) std::cerr << "GaitSym2019PythonLibrary object destroyed\n";
}

int GaitSym2019PythonLibrary::SetArguments(const std::vector<std::string> &argumentString)
{
    if (m_debug) std::cerr << "GaitSym2019PythonLibrary::SetArguments\n";
    std::string compileDate(__DATE__);
    std::string compileTime(__TIME__);
    int argc = int(argumentString.size());
    std::vector<const char *> argv;
    argv.reserve(argumentString.size());
    for (size_t i = 0; i < argumentString.size(); i++) argv.push_back(argumentString[i].c_str());
    m_argparse.Initialise(argc, argv.data(), "GaitSym2019 python interface to GaitSym2019 build "s + compileDate + " "s + compileTime, 0, 0);
    m_argparse.AddArgument("-sc"s, "--score"s, "Score filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-ow"s, "--outputWarehouse"s, "Output warehouse filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-iw"s, "--inputWarehouse"s, "Input warehouse filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-ms"s, "--modelState"s, "Model state filename"s, ""s, 1, false, ArgParse::String);
    m_argparse.AddArgument("-rt"s, "--runTimeLimit"s, "Run time limit"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-st"s, "--simulationTimeLimit"s, "Simulation time limit"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mc"s, "--outputModelStateAtCycle"s, "Output model state at this cycle"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mt"s, "--outputModelStateAtTime"s, "Output model state at this cycle"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-mw"s, "--outputModelStateAtWarehouseDistance"s, "Output model state at this warehouse distance"s, ""s, 1, false, ArgParse::Double);
    m_argparse.AddArgument("-wd"s, "--warehouseFailDistanceAbort"s, "Abort the simulation when the warehouse distance fails"s);
    m_argparse.AddArgument("-de"s, "--debug"s, "Turn debugging on"s);

    m_argparse.AddArgument("-ol"s, "--outputList"s, "List of objects to produce output"s, ""s, 1, MAX_ARGS, false, ArgParse::String);

    int err = m_argparse.Parse();
    if (err)
    {
        m_argparse.Usage();
        return __LINE__;
    }

    m_argparse.Get("--outputList"s, &m_outputList);
    m_argparse.Get("--runTimeLimit"s, &m_runTimeLimit);
    m_argparse.Get("--outputModelStateAtTime"s, &m_outputModelStateAtTime);
    m_argparse.Get("--outputModelStateAtCycle"s, &m_outputModelStateAtCycle);
    m_argparse.Get("--outputModelStateAtWarehouseDistance"s, &m_outputModelStateAtWarehouseDistance);
    m_argparse.Get("--simulationTimeLimit"s, &m_simulationTimeLimit);
    m_argparse.Get("--warehouseFailDistanceAbort"s, &m_warehouseFailDistanceAbort);
    m_argparse.Get("--config"s, &m_configFilename);
    m_argparse.Get("--score"s, &m_scoreFilename);
    m_argparse.Get("--modelState"s, &m_outputModelStateFilename);
    m_argparse.Get("--inputWarehouse"s, &m_inputWarehouseFilename);
    m_argparse.Get("--outputWarehouse"s, &m_outputWarehouseFilename);
    m_argparse.Get("--debug"s, &m_debug);

    return 0;
}

int GaitSym2019PythonLibrary::Run()
{
    if (m_debug) std::cerr << "GaitSym2019PythonLibrary::Run\n";
    // create the simulation object
    m_simulation = std::make_unique<Simulation>();
    if (m_outputWarehouseFilename.size()) m_simulation->SetOutputWarehouseFile(m_outputWarehouseFilename);
    if (m_outputModelStateFilename.size()) m_simulation->SetOutputModelStateFile(m_outputModelStateFilename);
    if (m_outputModelStateAtTime >= 0) m_simulation->SetOutputModelStateAtTime(m_outputModelStateAtTime);
    if (m_outputModelStateAtCycle >= 0) m_simulation->SetOutputModelStateAtCycle(m_outputModelStateAtCycle);
    if (m_inputWarehouseFilename.size()) m_simulation->AddWarehouse(m_inputWarehouseFilename);
    if (m_outputModelStateAtWarehouseDistance >= 0) m_simulation->SetOutputModelStateAtWarehouseDistance(m_outputModelStateAtWarehouseDistance);

    if (m_debug) std::cerr << "Loading model size = " << m_xmlData.size() << "\n";
    if (m_simulation->LoadModel(m_xmlData.data(), m_xmlData.size()))
    {
        std::cerr << "Error loading model\n";
        m_simulation.reset();
        return __LINE__;
    }
    if (m_debug) std::cerr << "Success\n";

    // late initialisation options
    if (m_simulationTimeLimit >= 0) m_simulation->SetTimeLimit(m_simulationTimeLimit);
    if (m_warehouseFailDistanceAbort != 0) m_simulation->SetWarehouseFailDistanceAbort(m_warehouseFailDistanceAbort);
    for (size_t i = 0; i < m_outputList.size(); i++)
    {
        if (m_simulation->GetBodyList()->find(m_outputList[i]) != m_simulation->GetBodyList()->end()) (*m_simulation->GetBodyList())[m_outputList[i]]->setDump(true);
        if (m_simulation->GetMuscleList()->find(m_outputList[i]) != m_simulation->GetMuscleList()->end()) (*m_simulation->GetMuscleList())[m_outputList[i]]->setDump(true);
        if (m_simulation->GetStrapList()->find(m_outputList[i]) != m_simulation->GetStrapList()->end()) (*m_simulation->GetStrapList())[m_outputList[i]]->setDump(true);
        if (m_simulation->GetGeomList()->find(m_outputList[i]) != m_simulation->GetGeomList()->end()) (*m_simulation->GetGeomList())[m_outputList[i]]->setDump(true);
        if (m_simulation->GetJointList()->find(m_outputList[i]) != m_simulation->GetJointList()->end()) (*m_simulation->GetJointList())[m_outputList[i]]->setDump(true);
        if (m_simulation->GetDriverList()->find(m_outputList[i]) != m_simulation->GetDriverList()->end()) (*m_simulation->GetDriverList())[m_outputList[i]]->setDump(true);
        if (m_simulation->GetDataTargetList()->find(m_outputList[i]) != m_simulation->GetDataTargetList()->end()) (*m_simulation->GetDataTargetList())[m_outputList[i]]->setDump(true);
        if (m_simulation->GetReporterList()->find(m_outputList[i]) != m_simulation->GetReporterList()->end()) (*m_simulation->GetReporterList())[m_outputList[i]]->setDump(true);
    }

    double startTime = GSUtil::GetTime();

    while(m_runTimeLimit <= 0 || m_simulationTime <= m_runTimeLimit)
    {
        if (m_debug > 2) std::cerr << "m_simulation->GetTime() = " << m_simulation->GetTime() << "\n";
        m_simulationTime = GSUtil::GetTime() - startTime;
        if (m_simulation->ShouldQuit()) break;
        if (m_simulation->TestForCatastrophy()) break;
        m_simulation->UpdateSimulation();
    }

    return 0;
}

// this routine attemps to read the model specification and initialise the simulation
// it returns zero on success
int GaitSym2019PythonLibrary::ReadModel(const std::string &configFilename)
{
    if (m_debug) std::cerr << "GaitSym2019PythonLibrary::ReadModel(\"" << configFilename << "\"\n";
    try
    {
        std::ifstream ifs(configFilename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        std::ifstream::pos_type fileSize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        m_xmlData.resize(fileSize);
        ifs.read(m_xmlData.data(), fileSize);
        return 0;
    }
    catch (...)
    {
        return __LINE__;
    }
}

void GaitSym2019PythonLibrary::SetXML(const std::string &xmlString)
{
    if (m_debug) std::cerr << "GaitSym2019PythonLibrary::SetXML\n";
    m_xmlData = xmlString;
}

double GaitSym2019PythonLibrary::GetFitness()
{
    if (m_debug) std::cerr << "GaitSym2019PythonLibrary::GetFitness\n";
    double score = -std::numeric_limits<double>::max();
    if (m_simulation) score = m_simulation->CalculateInstantaneousFitness();
    return score;
}
