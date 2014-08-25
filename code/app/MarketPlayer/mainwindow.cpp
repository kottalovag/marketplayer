#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "model.h"
#include "qcustomplot.h"

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

void MainWindow::plotEdgeworth(QCustomPlot* plot, const Simulation::EdgeworthSituation& situation) const {
    for (size_t graphIdx = 0; graphIdx < plot->plottableCount(); ++graphIdx) {
        plot->graph(graphIdx)->clearData();
    }
    Amount_t const q1RangeStart = std::numeric_limits<Amount_t>::epsilon();
    Amount_t const q1Resolution = 0.1;

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
}

void MainWindow::plotResourceDistribution(
        QCustomPlot* plot, Simulation const& simulation, size_t resourceIdx, Amount_t resolution) const
{
    plotDistribution(plot, simulation.resources[resourceIdx], resolution);
}

void MainWindow::plotUtilityDistribution(QCustomPlot* plot, Simulation const& simulation, Amount_t resolution) const
{
    plotDistribution(plot, simulation.computeUtilities(), resolution);
}

std::function<void(Position const&)> debugShowPoint;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    urng.seed(std::time(0));
    ui->setupUi(this);

    setupEdgeworthBox();
    setupDistributionPlot(ui->plotQ1Distribution, "Q1", "Actors");
    setupDistributionPlot(ui->plotQ2Distribution, "Q2", "Actors");
    setupDistributionPlot(ui->plotUtilityDistribution, "Utility", "Actors");

    Simulation simulation;
    simulation.setup(100, 10000, 8000, 0.5, 0.5);
    simulation.tradeStrategy.reset(new OppositeParetoTradeStrategy);

    //sample
    Simulation::EdgeworthSituation situation(simulation, 0, 1);
    plotEdgeworth(ui->plotEdgeworthBox, situation);
    debugShowPoint = [this](Position p){
        auto debugGraph = ui->plotEdgeworthBox->graph(5);
        debugGraph->addData(p.q1, p.q2);
    };
    RandomTriangleTradeStrategy strategy;
    strategy.propose(situation);

    //plot distributions
    Amount_t const q1Resolution = simulation.amounts[0] / simulation.numActors;
    plotResourceDistribution(ui->plotQ1Distribution, simulation, 0, q1Resolution);

    Amount_t const q2Resolution = simulation.amounts[1] / simulation.numActors;
    plotResourceDistribution(ui->plotQ2Distribution, simulation, 1, q2Resolution);

    Amount_t const utilityResolution = simulation.utility.compute(simulation.amounts[0],simulation.amounts[1])
            /simulation.numActors;
    plotUtilityDistribution(ui->plotUtilityDistribution, simulation, utilityResolution);
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
