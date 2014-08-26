#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "model.h"
#include "qcustomplot.h"

#include <iostream>
#include <time.h>

using std::cout;
using std::endl;

std::function<void(Position const&)> debugShowPoint;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    urng.seed(time(0));
    ui->setupUi(this);

    setupControlsStartup();

    setupEdgeworthBox();
    setupDistributionPlot(ui->plotQ1Distribution, "Q1", "Actors");
    setupDistributionPlot(ui->plotQ2Distribution, "Q2", "Actors");
    setupDistributionPlot(ui->plotUtilityDistribution, "Utility", "Actors");

    debugShowPoint = [this](Position p){
        auto debugGraph = ui->plotEdgeworthBox->graph(5);
        debugGraph->addData(p.q1, p.q2);
    };
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupControlsStartup()
{
    ui->lineEditNumActors->setText("100");
    ui->lineEditNumActors->setValidator(new QIntValidator(2,2000000000, this));

    auto resourceValidator = new QDoubleValidator(
                std::numeric_limits<Amount_t>::epsilon(), std::numeric_limits<double>::max(), 10, this);
    ui->lineEditSumQ1->setText("1000");
    ui->lineEditNumActors->setValidator(resourceValidator);
    ui->lineEditSumQ2->setText("800");
    ui->lineEditNumActors->setValidator(resourceValidator);

    auto alfaValidator = new QDoubleValidator(std::numeric_limits<Amount_t>::epsilon(), 1, 10, this);
    ui->lineEditAlfa1->setText("0.5");
    ui->lineEditAlfa1->setValidator(alfaValidator);
    ui->lineEditAlfa1->setEnabled(false); //TODO resolve
    ui->lineEditAlfa2->setText("0.5");
    ui->lineEditAlfa2->setValidator(alfaValidator);
    ui->lineEditAlfa2->setEnabled(false); //TODO resolve

    ui->radioButtonOppositePareto->setEnabled(true);

    connect(ui->sliderTime, SIGNAL(rangeChanged(int,int)),
            this, SLOT(onSliderTimeRangeChanged(int,int)));

    connect(ui->pushButtonApply, SIGNAL(clicked()),
            ui->actionApply, SLOT(trigger()));
    connect(ui->pushButtonNextRound, SIGNAL(clicked()),
            ui->actionNextRound, SLOT(trigger()));
    connect(ui->pushButtonStart, SIGNAL(clicked()),
            ui->actionStart, SLOT(trigger()));
    connect(ui->pushButtonPause, SIGNAL(clicked()),
            ui->actionPause, SLOT(trigger()));
    connect(ui->pushButtonNextTrade, SIGNAL(clicked()),
            ui->actionNextTrade, SLOT(trigger()));

    ui->sliderTime->setValue(0);

    timer = new QTimer(this);
    timer->setInterval(calculateSpeedInterval());
    connect(timer, SIGNAL(timeout()),
            ui->actionNextRound, SLOT(trigger()));

    resetControls();

    changeToTab(ui->tabWidget, ui->tabSetup);
}

int MainWindow::calculateSpeedInterval() const {
    auto const maxValue = ui->sliderSpeed->maximum();
    auto const maxTime = 1000;
    auto const value = ui->sliderSpeed->value();
    double ratio = (double)value/(double)maxValue;
    return maxTime * ratio;
}

void MainWindow::resetControls()
{
    ui->sliderTime->setMinimum(0);
    ui->sliderTime->setMaximum(0);

    cleanPlotData(ui->plotEdgeworthBox);
    cleanPlotData(ui->plotQ1Distribution);
    cleanPlotData(ui->plotQ2Distribution);
    cleanPlotData(ui->plotUtilityDistribution);
    cleanPlotData(ui->plotQ1Traded);
    cleanPlotData(ui->plotQ2Traded);

    ui->actionSaveEdgeworthDiagram->setEnabled(false);

    ui->actionNextRound->setEnabled(false);
    ui->pushButtonNextRound->setEnabled(false);
    ui->actionStart->setEnabled(false);
    ui->pushButtonStart->setEnabled(false);
    ui->actionPause->setEnabled(false);
    ui->pushButtonPause->setEnabled(false);
    ui->actionNextTrade->setEnabled(false);
    ui->pushButtonNextTrade->setEnabled(false);
}

void MainWindow::cleanPlotData(QCustomPlot* plot)
{
    for (size_t graphIdx = 0; graphIdx < plot->plottableCount(); ++graphIdx) {
        plot->graph(graphIdx)->clearData();
    }
}

void MainWindow::setupEdgeworthBox()
{
    auto plot = ui->plotEdgeworthBox;

    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)),
            plot->yAxis2, SLOT(setRange(QCPRange)));

    plot->xAxis2->setVisible(true);
    plot->yAxis2->setVisible(true);
    plot->xAxis2->setRangeReversed(true);
    plot->yAxis2->setRangeReversed(true);

    plot->xAxis->setLabel("Actor 1's Q1");
    plot->yAxis->setLabel("Actor 1's Q2");
    plot->xAxis2->setLabel("Actor 2's Q1");
    plot->yAxis2->setLabel("Actor 2's Q2");

    //curve1
    plot->addGraph();

    //curve2
    plot->addGraph();

    //pareto-set
    plot->addGraph();
    auto paretoSet = plot->graph(2);
    paretoSet->setPen(QPen(Qt::darkGreen));

    //points
    plot->addGraph();
    auto pointsGraph = plot->graph(3);
    pointsGraph->setLineStyle(QCPGraph::lsNone);
    pointsGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross));
    auto pointsPen = pointsGraph->pen();
    pointsPen.setColor(Qt::red);
    pointsPen.setWidth(2);
    pointsGraph->setPen(pointsPen);

    //result
    plot->addGraph();
    auto resultGraph = plot->graph(4);
    resultGraph->setLineStyle(QCPGraph::lsNone);
    resultGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
    auto resultPen = resultGraph->pen();
    resultPen.setColor(Qt::darkGray);
    resultPen.setWidth(2);
    resultGraph->setPen(resultPen);

    //debug
    plot->addGraph();
    auto debugGraph = plot->graph(5);
    debugGraph->setLineStyle(QCPGraph::lsNone);
    debugGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
    auto debugPen = debugGraph->pen();
    debugPen.setColor(Qt::cyan);
    debugPen.setWidth(2);
    debugGraph->setPen(debugPen);
}

void MainWindow::plotEdgeworth(QCustomPlot* plot, Simulation::EdgeworthSituation const& situation) {
    cleanPlotData(plot);

    Amount_t const q1RangeStart = std::numeric_limits<Amount_t>::epsilon();
    Amount_t const q1Resolution = situation.q1Sum / 4000; //todo generalize

    plot->xAxis->setRange(0.0, situation.q1Sum);
    plot->yAxis->setRange(0.0, situation.q2Sum);

    auto curve1Graph = plot->graph(0);
    ResourceDataPair data1 = sampleFunction(
                situation.getCurve1Function(), q1RangeStart, situation.q1Sum, q1Resolution);
    curve1Graph->setData(data1.x, data1.y);
    //ResourceDataPair data1 = sampleFunction(situation.getCurve1Function(), situation.actor1.q1, situation.q1Sum, q1Resolution);
    /*auto graph1Brush = graph1->brush();
            graph1Brush.setStyle(Qt::SolidPattern);
            QColor fillColor = Qt::blue;
            fillColor.setAlpha(32);
            graph1Brush.setColor(fillColor);
            graph1->setBrush(graph1Brush);
            graph1->setChannelFillGraph(plot->graph(1));*/

    auto curve2Graph = plot->graph(1);
    ResourceDataPair data2 = sampleFunction(situation.getCurve2Function(),
                                            q1RangeStart, situation.q1Sum, q1Resolution);
    curve2Graph->setData(data2.x, data2.y);

    auto paretoSet = plot->graph(2);
    ResourceDataPair paretoData = sampleFunction(situation.getParetoSetFunction(),
                                                 q1RangeStart, situation.q1Sum, q1Resolution);
    paretoSet->setData(paretoData.x, paretoData.y);

    auto pointsGraph = plot->graph(3);
    auto fixPoint = situation.getFixPoint();
    pointsGraph->addData(fixPoint.q1, fixPoint.q2);

    auto p1Point = situation.calculateCurve1ParetoIntersection();
    pointsGraph->addData(p1Point.q1, p1Point.q2);

    auto p2Point = situation.calculateCurve2ParetoIntersection();
    pointsGraph->addData(p2Point.q1, p2Point.q2);

    auto resultGraph = plot->graph(4);
    resultGraph->addData(situation.result.q1, situation.result.q2);

    plot->replot();
}

void MainWindow::setupDistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel)
{
    auto bars = new QCPBars(plot->xAxis, plot->yAxis);
    plot->addPlottable(bars);
    plot->xAxis->setLabel(xLabel);
    plot->yAxis->setLabel(yLabel);

    /*QBrush brush;
    color.setAlpha(brush.color().alpha());
    brush.setColor(color);
    brush.setStyle(Qt::SolidPattern);
    bars->setBrush(brush);*/
}

void MainWindow::plotDistribution(QCustomPlot* plot, const vector<Amount_t>& subject, Amount_t resolution)
{
    Distribution const distribution(subject, resolution);
    auto const& data = distribution.data;

    auto bars = static_cast<QCPBars*>(plot->plottable(0));
    bars->setWidth(resolution);
    bars->setData(data.x, data.y);
    plot->xAxis->setRange(0.0, data.x.last()+resolution);
    plot->yAxis->setRange(0.0, *std::max_element(data.y.begin(), data.y.end()) + 1.0);
    plot->xAxis->setTickStep(resolution);

    plot->replot();
}

void MainWindow::plotResourceDistribution(
        QCustomPlot* plot, Simulation const& simulation, size_t resourceIdx, Amount_t resolution) const
{
    plotDistribution(plot, simulation.resources[resourceIdx], resolution);
}

void MainWindow::plotUtilityDistribution(QCustomPlot* plot, Simulation const& simulation, Amount_t resolution) const
{
    auto const& utilities = simulation.computeUtilities();
    plotDistribution(plot, utilities, resolution);
    //todo move this hack to history of the model
    auto sum = std::accumulate(utilities.begin(), utilities.end(), 0.0);
    ui->labelSumUtilities->setText("Sum: " + QString::number(sum));
}

void MainWindow::updateOverview()
{
    //todo generalize
    Amount_t const q1Resolution = simulation.amounts[0] / simulation.numActors / 8;
    plotResourceDistribution(ui->plotQ1Distribution, simulation, 0, q1Resolution);

    Amount_t const q2Resolution = simulation.amounts[1] / simulation.numActors / 8;
    plotResourceDistribution(ui->plotQ2Distribution, simulation, 1, q2Resolution);

    Amount_t const utilityResolution = simulation.utility.compute(simulation.amounts[0],simulation.amounts[1])
            /simulation.numActors / 8;
    plotUtilityDistribution(ui->plotUtilityDistribution, simulation, utilityResolution);
}

void MainWindow::updateTime()
{
    size_t currentTimeUnit = simulation.history.time;
    ui->sliderTime->setMaximum(currentTimeUnit);
    ui->sliderTime->setValue(currentTimeUnit);
    ui->spinBoxTime->setValue(currentTimeUnit);
}

void MainWindow::plotNextSituation()
{
    Simulation::EdgeworthSituation nextSituation = simulation.getNextSituation();
    plotEdgeworth(ui->plotEdgeworthBox, nextSituation);
}

void MainWindow::updateProgress()
{
    ui->labelProgress->setText(QString::number(simulation.progress.getDone())
                               + "/"
                               + QString::number(simulation.progress.getNum()));
    ui->progressBarRound->setValue(simulation.progress.getDone());
}

void MainWindow::onSliderTimeRangeChanged(int min, int max)
{
    if (ui->spinBoxTime->minimum() != min) {
        ui->spinBoxTime->setMinimum(min);
    }
    if (ui->spinBoxTime->maximum() != max) {
        ui->spinBoxTime->setMaximum(max);
    }
}

void MainWindow::on_actionSaveEdgeworthDiagram_triggered()
{
    auto now = QDateTime::currentDateTime();
    auto fileName = "Edgeworth_" + now.toString("yyyy.MM.dd_hh.mm.ss") + ".png";
    ui->plotEdgeworthBox->savePng(fileName);
}

void MainWindow::changeToTab(QTabWidget* tabWidget, QWidget* desiredTab)
{
    size_t const tabIdx = ui->tabWidget->indexOf(desiredTab);
    tabWidget->setCurrentIndex(tabIdx);
}

void MainWindow::on_actionApply_triggered()
{
    on_actionPause_triggered();

    simulation.setup(ui->lineEditNumActors->text().toInt(),
                     ui->lineEditSumQ1->text().toDouble(),
                     ui->lineEditSumQ2->text().toDouble(),
                     ui->lineEditAlfa1->text().toDouble(),
                     ui->lineEditAlfa2->text().toDouble());

    unique_ptr<AbstractTradeStrategy> strategy;
    if (ui->radioButtonOppositePareto->isChecked()) {
        strategy.reset(new OppositeParetoTradeStrategy);
    } else if (ui->radioButtonRandomPareto->isChecked()) {
        strategy.reset(new RandomParetoTradeStrategy);
    } else {
        strategy.reset(new RandomTriangleTradeStrategy);
    }
    simulation.tradeStrategy = std::move(strategy);

    ui->progressBarRound->setMaximum(simulation.progress.getNum());

    plotNextSituation();
    updateOverview();
    updateProgress();
    updateTime();

    changeToTab(ui->tabWidget, ui->tabOverview);
}

void MainWindow::on_actionNextRound_triggered()
{
    auto tabIdx = ui->tabWidget->currentIndex();
    if ( ui->tabWidget->indexOf(ui->tabEdgeworthBox) == tabIdx ) {
        while(!simulation.progress.wasRestarted()) {
            on_actionNextTrade_triggered();
        }
    } else {
        simulation.performNextRound();
        plotNextSituation();
    }
    updateTime();
    updateProgress();
    updateOverview();
}

void MainWindow::on_actionStart_triggered()
{
    timer->start();

    ui->actionStart->setEnabled(false);
    ui->pushButtonStart->setEnabled(false);

    ui->actionPause->setEnabled(true);
    ui->pushButtonPause->setEnabled(true);

    ui->actionNextTrade->setEnabled(false);
    ui->pushButtonNextTrade->setEnabled(false);

    ui->actionSaveEdgeworthDiagram->setEnabled(false);
}

void MainWindow::on_actionPause_triggered()
{
    timer->stop();

    ui->actionNextRound->setEnabled(true);
    ui->pushButtonNextRound->setEnabled(true);

    ui->actionStart->setEnabled(true);
    ui->pushButtonStart->setEnabled(true);

    ui->actionPause->setEnabled(false);
    ui->pushButtonPause->setEnabled(false);

    ui->actionNextTrade->setEnabled(true);
    ui->pushButtonNextTrade->setEnabled(true);

    ui->actionSaveEdgeworthDiagram->setEnabled(true);
}

void MainWindow::on_actionNextTrade_triggered()
{
    bool const isFinished = simulation.performNextTrade();
    updateProgress();
    if (isFinished) {
        updateTime();
    }
    plotNextSituation();
}

void MainWindow::on_sliderSpeed_valueChanged(int)
{
    timer->setInterval(calculateSpeedInterval());
}
