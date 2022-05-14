#ifndef GAITSYM2019PYTHONLIBRARY_H
#define GAITSYM2019PYTHONLIBRARY_H

#include "GaitSym2019PythonLibrary_global.h"

#include "XMLConverter.h"
#include "ArgParse.h"

#include <string>
#include <vector>
#include <memory>

class Simulation;

class GAITSYM2019PYTHONLIBRARY_EXPORT GaitSym2019PythonLibrary
{
public:
    GaitSym2019PythonLibrary();
    ~GaitSym2019PythonLibrary();

    int SetArguments(const std::vector<std::string> &argumentString);
    int ReadModel(const std::string &configFilename);
    void SetXML(const std::string &xmlString);
    int Run();

    double GetFitness();

private:


    std::vector<std::string> m_outputList;

    std::unique_ptr<Simulation> m_simulation;
    double m_runTimeLimit = 0;
    double m_simulationTime = 0;
    double m_outputModelStateAtTime = -1;
    double m_outputModelStateAtCycle = -1;
    double m_outputModelStateAtWarehouseDistance = -1;
    double m_simulationTimeLimit = -1;
    double m_warehouseFailDistanceAbort = 0;

    std::string m_configFilename;
    std::string m_outputWarehouseFilename;
    std::string m_outputModelStateFilename;
    std::string m_inputWarehouseFilename;
    std::string m_scoreFilename;

    std::string m_xmlData;

    XMLConverter m_XMLConverter;
    ArgParse m_argparse;
    int m_debug = 2;
};

#endif // GAITSYM2019PYTHONLIBRARY_H

