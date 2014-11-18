#ifndef APPINSIMULATIONMODE_H
#define APPINSIMULATIONMODE_H

#include "appstate.h"

struct AppInSimulationMode : AppState
{
    AppInSimulationMode(MainWindow* mainWindow);

    virtual void enter() override;
    virtual void beforeSimulationSetup() override;
    virtual void comparisonModeSelected() override;
    virtual void handleCaseAddition() override;
private:
    void stopSimulation();
};

#endif // APPINSIMULATIONMODE_H
