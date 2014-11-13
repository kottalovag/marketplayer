#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "model.h"
#include "datatimeplot.h"
#include "datatimeplotwithpercentage.h"
#include "distributionplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void changeToTab(QTabWidget* tabWidget, QWidget* desiredTab);
    void setupSpeedControls();

private:
    int calculateSpeedInterval() const;
    void setupControlsStartup();
    void resetControls();

    void setupEdgeworthBox();
    void plotEdgeworth(QCustomPlot* plot, Simulation::EdgeworthSituation const& situation);
    void plotNextSituation();

    void updateOverview();
    void loadHistoryMoment(int time);
    void updateTimeRange();
    void updateProgress();

private slots:
    void onSliderTimeRangeChanged(int min, int max);

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

private:
    Ui::MainWindow *ui;
    Simulation simulation;
    QTimer* timer;
    unique_ptr<DataTimePlotWithPercentage> plotQ1Traded;
    unique_ptr<DataTimePlotWithPercentage> plotQ2Traded;
    unique_ptr<DataTimePlot> plotSumUtility;
    unique_ptr<DataTimePlotWithPercentage> plotNumSuccessfulTrades;
    unique_ptr<DataTimePlot> plotWealthDeviation;

    unique_ptr<DistributionPlot> plotQ1Distribution;
    unique_ptr<DistributionPlot> plotQ2Distribution;
    unique_ptr<DistributionPlot> plotUtilityDistribution;
    unique_ptr<DistributionPlot> plotWealthDistribution;
};

#endif // MAINWINDOW_H




