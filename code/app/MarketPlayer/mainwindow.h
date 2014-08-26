#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "model.h"

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
private:
    int calculateSpeedInterval() const;
    void setupControlsStartup();
    void resetControls();

    void setupEdgeworthBox();
    void setupDistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel);

    void plotEdgeworth(QCustomPlot* plot, Simulation::EdgeworthSituation const& situation);

    static void plotDistribution(QCustomPlot* plot, vector<Amount_t> const& subject, Amount_t resolution);

    void plotResourceDistribution(QCustomPlot* plot, const Simulation& simulation, size_t resourceIdx, Amount_t resolution) const;
    void plotUtilityDistribution(QCustomPlot* plot, const Simulation& simulation, Amount_t resolution) const;

    void cleanPlotData(QCustomPlot* plot);
    void updateOverview();
    void updateTime();
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

private:
    Ui::MainWindow *ui;
    Simulation simulation;
    QTimer* timer;
};

#endif // MAINWINDOW_H
