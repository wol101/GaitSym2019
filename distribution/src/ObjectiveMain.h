#ifndef OBJECTIVEMAIN_H
#define OBJECTIVEMAIN_H

#include "XMLConverter.h"
#include "ArgParse.h"

#include <string>
#include <vector>
#include <memory>

class Simulation;

class ObjectiveMain
{
public:
    ObjectiveMain(int argc, const char **argv);

    int Run();
    int ReadModel();
    int WriteOutput();

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

    XMLConverter m_XMLConverter;
    ArgParse m_argparse;
    bool m_debug = false;
};

#endif // OBJECTIVEMAIN_H
