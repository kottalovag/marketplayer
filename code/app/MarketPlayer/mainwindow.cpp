#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "model.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    urng.seed(std::time(0));
    ui->setupUi(this);

    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->plotEdgeworthBox->xAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotEdgeworthBox->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plotEdgeworthBox->yAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotEdgeworthBox->yAxis2, SLOT(setRange(QCPRange)));
    ui->plotEdgeworthBox->xAxis2->setVisible(true);
    ui->plotEdgeworthBox->yAxis2->setVisible(true);
    ui->plotEdgeworthBox->xAxis2->setRangeReversed(true);
    ui->plotEdgeworthBox->yAxis2->setRangeReversed(true);

    Simulation simulation;
    simulation.setup(10, 1000, 800, 0.5, 0.5);
    simulation.printResources(0);
    simulation.printResources(1);
    Simulation::EdgeworthSituation situation(simulation, 0, 1);
    simulation.plotEdgeworth(ui->plotEdgeworthBox, situation);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionSaveEdgeworthDiagram_triggered()
{
    auto now = QDateTime::currentDateTime();
    auto fileName = "Edgeworth_" + now.toString("yyyy.MM.dd_hh.mm.ss") + ".png";
    ui->plotEdgeworthBox->savePng(fileName);
}
