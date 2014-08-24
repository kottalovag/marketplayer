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

private:
    void setupEdgeworthBox();
    void plotEdgeworth(QCustomPlot* plot, const Simulation::EdgeworthSituation& situation) const;

    static void plotDistribution(QCustomPlot* plot, vector<Amount_t> const& subject, Amount_t resolution);
    void setupDistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel);
    void plotResourceDistribution(QCustomPlot* plot, const Simulation& simulation, size_t resourceIdx, Amount_t resolution) const;
    void plotUtilityDistribution(QCustomPlot* plot, const Simulation& simulation, Amount_t resolution) const;

private slots:
    void on_actionSaveEdgeworthDiagram_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
