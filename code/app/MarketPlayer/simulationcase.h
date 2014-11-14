#ifndef SIMULATIONCASE_H
#define SIMULATIONCASE_H

#include "model.h"
#include <QColor>

struct SimulationCase
{
    Simulation simulation;
    QColor color;
    bool isShown;
};

#endif // SIMULATIONCASE_H
