#include "simulationcase.h"

AbstractSimulationCase::AbstractSimulationCase(QString caseName, QColor color, bool visible)
    :   caseName(caseName)
    ,   color(color)
    ,   isShown(visible)
{
}

HeavySimulationCase::HeavySimulationCase(const Simulation &simulation, QString caseName, QColor color, bool visible)
    :   AbstractSimulationCase(caseName, color, visible)
    ,   simulation(simulation)
{
}

const Simulation &HeavySimulationCase::getSimulation() const {
    return simulation;
}

ExternalSimulationCase::ExternalSimulationCase(const Simulation &simulation, QString caseName, QColor color, bool visible)
    :   AbstractSimulationCase(caseName, color, visible)
    ,   simulation(&simulation)
{
}

const Simulation &ExternalSimulationCase::getSimulation() const {
    return *simulation;
}
