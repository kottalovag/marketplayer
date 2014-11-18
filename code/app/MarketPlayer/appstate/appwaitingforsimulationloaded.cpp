#include "appwaitingforsimulationloaded.h"
#include "mainwindow.h"

AppWaitingForSimulationLoaded::AppWaitingForSimulationLoaded(MainWindow *mainWindow)
    :   AppState(mainWindow)
{}

void AppWaitingForSimulationLoaded::simulationSetupOccured()
{
    mainWindow->setState(mainWindow->appInSimulationMode.get());
}
