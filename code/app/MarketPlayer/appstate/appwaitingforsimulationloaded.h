#ifndef APPWAITINGFORSIMULATIONLOADED_H
#define APPWAITINGFORSIMULATIONLOADED_H

#include "appstate.h"

struct AppWaitingForSimulationLoaded : AppState
{
    AppWaitingForSimulationLoaded(MainWindow* mainWindow);

    virtual void simulationSetupOccured() override;
};

#endif // APPWAITINGFORSIMULATIONLOADED_H
