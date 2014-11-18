#ifndef APPINCOMPARISONMODE_H
#define APPINCOMPARISONMODE_H

#include "appstate.h"

struct AppInComparisonMode : AppState
{
    AppInComparisonMode(MainWindow* mainWindow);

    virtual void enter() override;
    virtual void simulationSetupOccured() override;
    virtual void simulationModeSelected() override;
    virtual void handleCaseAddition() override;
    virtual void handleCaseRowVisibility(int rowIdx) override;
};

#endif // APPINCOMPARISONMODE_H
