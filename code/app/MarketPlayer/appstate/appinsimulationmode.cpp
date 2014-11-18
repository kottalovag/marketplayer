#include "appinsimulationmode.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

AppInSimulationMode::AppInSimulationMode(MainWindow *mainWindow)
    :   AppState(mainWindow)
{}

void AppInSimulationMode::enter()
{
    mainWindow->ui->radioButtonSimulation->setChecked(true);
    for (auto& action : {
         mainWindow->ui->actionNextRound,
         mainWindow->ui->actionNextTrade,
         mainWindow->ui->actionStart,
         mainWindow->ui->actionSaveConfiguration,
         mainWindow->ui->actionSaveEdgeworthDiagram
        })
    {
        action->setEnabled(true);
    }
}

void AppInSimulationMode::beforeSimulationSetup()
{
    stopSimulation();
}

void AppInSimulationMode::comparisonModeSelected()
{
    stopSimulation();
    if (mainWindow->getNumCaseRows() > 0) {
        mainWindow->caseManager->setVisibility(MainWindow::mainSimulationID, false);
        mainWindow->setupShownHistoryCases();
        mainWindow->updateTimeRange(mainWindow->caseManager->calculateLastVisibleDataIdx());
        mainWindow->setSelectedTimeIdx(mainWindow->caseManager->getValidCurrentDataIdx());

        mainWindow->setState(mainWindow->appInComparisonMode.get());
    } else {
        this->enter();
    }
}

void AppInSimulationMode::handleCaseAddition()
{
    mainWindow->addCaseRow(false);
}

void AppInSimulationMode::stopSimulation()
{
    mainWindow->on_actionPause_triggered();
}
