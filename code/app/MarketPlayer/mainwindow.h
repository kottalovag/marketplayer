#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "model.h"
#include "datatimeplot.h"
#include "datatimeplotwithpercentage.h"

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
    void setupDistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel);
    void plotEdgeworth(QCustomPlot* plot, Simulation::EdgeworthSituation const& situation);

    static void plotDistribution(QCustomPlot* plot, HeavyDistribution const& distribution);

    void plotResourceDistribution(QCustomPlot* plot, const Simulation& simulation, size_t resourceIdx, Amount_t resolution) const;
    void plotUtilityDistribution(QCustomPlot* plot, const Simulation& simulation, Amount_t resolution) const;

    void updateOverview();
    void loadHistoryMoment(int time);
    void updateTimeRange();
    void plotNextSituation();
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
};

#endif // MAINWINDOW_H




