#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <map>
#include <functional>

#include "model.h"
#include "datatimeplot.h"
#include "datatimeratioplot.h"
#include "distributionplot.h"
#include "simulationcase.h"
#include "colormanager.h"
#include "casenamemanager.h"
#include "casemanager.h"
#include "appstate.h"
#include "appwaitingforsimulationloaded.h"
#include "appinsimulationmode.h"
#include "appincomparisonmode.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend struct AppState;
    friend struct AppWaitingForSimulationLoaded;
    friend struct AppInSimulationMode;
    friend struct AppInComparisonMode;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void changeToTab(QTabWidget* tabWidget, QWidget* desiredTab);
    void setupSpeedControls();

private:
    void setState(AppState* nextState);

    int calculateSpeedInterval() const;
    void setupControlsAndUIStartup();
    void unmarkParameterControls();
    void applyUIToApplicationStarted();
    void applyUIToSimulationSetup();

    void setupEdgeworthBox();
    void plotEdgeworth(QCustomPlot* plot, EdgeworthSituation const& situation);
    void plotNextSituation();

    bool trySetupSimulationByForm();
    void setupSimulationByHistory(AbstractSimulationCase const& simulationCase);

    void updateParameterControlsFromSimulation(Simulation const& simulation);

    void loadHistoryMoment(int time);
    void updateTimeRange(size_t maxTimeIdx);
    void updateTimeRangeBySimulation();
    void setSelectedTimeIdx(size_t timeIdx);
    void updateProgress();

    void updateCaseInput();
    QColor getButtonColor(QPushButton* button) const;
    void setButtonColor(QPushButton* button, QColor color);

    void addCaseRow(bool visible);
    void removeCaseRows(std::function<bool(int)> pred);
    size_t getNumCaseRows() const;
    QString getCaseNameFromRow(size_t rowIdx) const;
    bool isCaseRowSelected(size_t rowIdx) const;
    bool isCaseRowChecked(size_t rowIdx) const;
    int getFirstSelectedSimulationCaseRow() const;
    void setupShownHistoryCases();

    void markLineEditChanged(QLineEdit* lineEdit, bool marked);
    void markGroupBoxChanged(QGroupBox* groupBox, bool marked);

private slots:
    void onSliderTimeRangeChanged(int min, int max);
    void on_buttonGroupOfferStrategy_buttonClicked(int buttonID);
    void on_buttonGroupAcceptanceStrategy_buttonClicked(int buttonID);
    void onButtonGroupOverviewMode_buttonClicked(int buttonID);

    void on_actionSaveEdgeworthDiagram_triggered();

    void on_actionApply_triggered();

    void on_actionStart_triggered();

    void on_actionPause_triggered();

    void on_actionNextTrade_triggered();

    void on_actionNextRound_triggered();

    void on_sliderSpeed_valueChanged(int value);

    void on_sliderTime_valueChanged(int value);

    void on_pushButtonRegenerateSeed_clicked();

    void on_actionSaveConfiguration_triggered();

    void on_actionLoadConfiguration_triggered();

    void on_pushButtonClearHistory_clicked();

    void on_pushButtonAddCurrentOutput_clicked();

    void on_tableWidgetCases_cellChanged(int row, int column);

    void on_pushButtonCaseColor_clicked();

    void on_pushButtonDeleteSelectedOutput_clicked();

    void on_pushButtonLoadSelectedOutput_clicked();

    void on_lineEditNumActors_textChanged(const QString &arg1);

    void on_lineEditSumQ1_textChanged(const QString &arg1);

    void on_lineEditSumQ2_textChanged(const QString &arg1);

    void on_lineEditAlfa1_textChanged(const QString &arg1);

    void on_lineEditAlfa2_textChanged(const QString &arg1);

    void on_lineEditSeed_textChanged(const QString &arg1);

    void on_tableWidgetCases_itemSelectionChanged();

    void on_actionRevertChanges_triggered();

private:
    AppState* currentState;
    unique_ptr<AppWaitingForSimulationLoaded> appWaitingForSimulationLoaded;
    unique_ptr<AppInSimulationMode> appInSimulationMode;
    unique_ptr<AppInComparisonMode> appInComparisonMode;

    Mode mode;

    //the window handles ownership:
    Ui::MainWindow *ui;
    QTimer* timer;
    //

    unique_ptr<CaseManager> caseManager;
    Simulation simulation;

    unique_ptr<DataTimeRatioPlot> plotQ1Traded;
    unique_ptr<DataTimeRatioPlot> plotQ2Traded;
    unique_ptr<DataTimePlot> plotSumUtility;
    unique_ptr<DataTimeRatioPlot> plotNumSuccessfulTrades;
    unique_ptr<DataTimePlot> plotWealthDeviation;

    unique_ptr<DistributionPlot> plotQ1Distribution;
    unique_ptr<DistributionPlot> plotQ2Distribution;
    unique_ptr<DistributionPlot> plotUtilityDistribution;
    unique_ptr<DistributionPlot> plotWealthDistribution;

    std::map<QString, QRadioButton*> strategyMap;

    ColorManager colorManager;
    CaseNameManager caseNameManager;
    static const int caseNameColumnIdx = 0;
    static const int caseColorColumnIdx = 1;
    static const int checkBoxColumnIdx = 2;

    static constexpr double defaultAlfa1 = 0.5;
    static constexpr double defaultAlfa2 = 0.5;

    static const QString mainSimulationID;
};

#endif // MAINWINDOW_H




