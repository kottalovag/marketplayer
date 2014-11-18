#ifndef APPSTATES_H
#define APPSTATES_H

class MainWindow;

enum class Mode;

struct AppState
{
    AppState(MainWindow* mainWindow);
    virtual ~AppState(){}

    virtual void enter(){}

    virtual void beforeSimulationSetup(){}
    virtual void simulationModeSelected(){}
    virtual void comparisonModeSelected(){}
    virtual void simulationSetupOccured(){}
    virtual void handleCaseAddition(){}
    virtual void handleCaseRowVisibility(int){}

protected:
    MainWindow* mainWindow;
};

#endif // APPSTATES_H
