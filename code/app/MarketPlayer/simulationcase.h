#ifndef SIMULATIONCASE_H
#define SIMULATIONCASE_H

#include "model.h"
#include <QColor>

struct AbstractSimulationCase
{
    AbstractSimulationCase(QString caseName, QColor color, bool visible);
    virtual Simulation const& getSimulation() const = 0;
    virtual ~AbstractSimulationCase(){}

    QString caseName;
    QColor color;
    bool isShown;    
};

struct HeavySimulationCase : AbstractSimulationCase
{
    HeavySimulationCase(Simulation const& simulation, QString caseName, QColor color, bool visible);
    virtual Simulation const& getSimulation() const;

private:
    Simulation simulation;
};

struct ExternalSimulationCase : AbstractSimulationCase
{
    ExternalSimulationCase(Simulation const& simulation, QString caseName, QColor color, bool visible);
    virtual Simulation const& getSimulation() const;

private:
    Simulation const* simulation;
};

#endif // SIMULATIONCASE_H
