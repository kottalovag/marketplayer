#include "appincomparisonmode.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

AppInComparisonMode::AppInComparisonMode(MainWindow *mainWindow)
    :   AppState(mainWindow)
{}

void AppInComparisonMode::enter()
{
    mainWindow->on_actionPause_triggered();
    mainWindow->ui->radioButtonComparison->setChecked(true);
    for (auto& action : {
         mainWindow->ui->actionNextRound,
         mainWindow->ui->actionNextTrade,
         mainWindow->ui->actionStart
        })
    {
        action->setEnabled(false);
    }
}

void AppInComparisonMode::simulationSetupOccured()
{
    simulationModeSelected();
}

void AppInComparisonMode::simulationModeSelected()
{
    mainWindow->caseManager->hideAllCases();
    mainWindow->caseManager->setVisibility(MainWindow::mainSimulationID, true);
    mainWindow->updateTimeRangeBySimulation();

    mainWindow->setState(mainWindow->appInSimulationMode.get());
}

void AppInComparisonMode::handleCaseAddition()
{
    mainWindow->addCaseRow(true);
}

void AppInComparisonMode::handleCaseRowVisibility(int rowIdx)
{
    mainWindow->caseManager->setVisibility(
                mainWindow->getCaseNameFromRow(rowIdx),
                mainWindow->isCaseRowChecked(rowIdx));

}
