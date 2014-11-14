#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "model.h"
#include "qcustomplot.h"
#include "plotutils.h"

#include <iostream>
#include <time.h>
#include <QFileDialog>
#include <QMap>

using std::cout;
using std::endl;

std::function<void(Position const&)> debugShowPoint;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    globalUrng.seed(time(0));
    ui->setupUi(this);

    setupControlsStartup();

    setupEdgeworthBox();

    plotQ1Traded.reset(new DataTimePlotWithPercentage(
        ui->plotQ1Traded, ui->labelQ1Traded, "Q1 traded", "Q1 traded"));
    plotQ2Traded.reset(new DataTimePlotWithPercentage(
        ui->plotQ2Traded, ui->labelQ2Traded, "Q2 traded", "Q2 traded"));
    plotSumUtility.reset(new DataTimePlot(
        ui->plotSumUtility, ui->labelSumUtilities, "Sum of utilities", "Sum of utilities"));
    plotNumSuccessfulTrades.reset(new DataTimePlotWithPercentage(
        ui->plotNumSuccessfulTrades, ui->labelNumSuccessful, "Successful trades", "Successful trades"));
    plotWealthDeviation.reset(new DataTimePlot(
        ui->plotWealthDeviation, ui->labelWealthDeviation, "Wealth deviation", "Wealth deviation"));

    plotQ1Distribution.reset(new DistributionPlot(
        ui->plotQ1Distribution, "Q1", "Actors"));
    plotQ2Distribution.reset(new DistributionPlot(
        ui->plotQ2Distribution, "Q2", "Actors"));
    plotUtilityDistribution.reset(new DistributionPlot(
        ui->plotUtilityDistribution, "Utiltiy", "Actors"));
    plotWealthDistribution.reset(new DistributionPlot(
        ui->plotWealthDistribution, "Wealth", "Actors"));

    resetControls();

    debugShowPoint = [this](Position p){
        auto debugGraph = ui->plotEdgeworthBox->graph(5);
        debugGraph->addData(p.q1, p.q2);
    };
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSpeedControls()
{
    timer = new QTimer(this);
    timer->setInterval(calculateSpeedInterval());
    connect(timer, SIGNAL(timeout()),
            ui->actionNextRound, SLOT(trigger()));

    ui->sliderSpeed->setMaximum(8);
    ui->sliderSpeed->setValue(2);
}

void MainWindow::setupControlsStartup()
{
    ui->lineEditNumActors->setText("1000");
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

    ui->lineEditSeed->setValidator(new QIntValidator(0,std::numeric_limits<unsigned>::max(), this));
    on_pushButtonRegenerateSeed_clicked();

    ui->radioButtonOppositePareto->setEnabled(true);
    ui->radioButtonWantAlways->setEnabled(true);

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

    setupSpeedControls();

    changeToTab(ui->tabWidget, ui->tabSetup);

    updateCaseInput();
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

    plotQ1Traded->reset();
    plotQ2Traded->reset();
    plotSumUtility->reset();
    plotWealthDeviation->reset();
    plotNumSuccessfulTrades->reset();

    plotQ1Distribution->reset();
    plotQ2Distribution->reset();
    plotUtilityDistribution->reset();
    plotWealthDistribution->reset();

    cleanPlotData(ui->plotEdgeworthBox);

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

void MainWindow::setupSimulationByForm()
{
    simulation.setup(ui->lineEditSeed->text().toUInt(),
                     ui->lineEditNumActors->text().toInt(),
                     ui->lineEditSumQ1->text().toDouble(),
                     ui->lineEditSumQ2->text().toDouble(),
                     ui->lineEditAlfa1->text().toDouble(),
                     ui->lineEditAlfa2->text().toDouble());

    unique_ptr<AbstractOfferStrategy> offerStrategy;
    if (ui->radioButtonOppositePareto->isChecked()) {
        offerStrategy.reset(new OppositeParetoOfferStrategy);
    } else if (ui->radioButtonRandomPareto->isChecked()) {
        offerStrategy.reset(new RandomParetoOfferStrategy);
    } else {
        offerStrategy.reset(new RandomTriangleOfferStrategy);
    }
    simulation.offerStrategy = std::move(offerStrategy);

    unique_ptr<AbstractAcceptanceStrategy> acceptanceStrategy;
    if (ui->radioButtonWantAlways->isChecked()) {
        acceptanceStrategy.reset(new AlwaysAcceptanceStrategy);
    } else if (ui->radioButtonWantHigherGain->isChecked()) {
        acceptanceStrategy.reset(new HigherGainAcceptanceStrategy);
    } else {
        acceptanceStrategy.reset(new HigherProportionAcceptanceStrategy);
    }
    simulation.acceptanceStrategy = std::move(acceptanceStrategy);
}

void MainWindow::updateOverview()
{
    loadHistoryMoment(simulation.history.time - 1);
}

void MainWindow::loadHistoryMoment(int time)
{
    auto const momentIdx = time;
    auto const& history = simulation.history;

    Moment const& moment = history.moments[momentIdx];
    plotQ1Distribution->plotData(moment.q1Distribution);
    plotQ2Distribution->plotData(moment.q2Distribution);
    plotUtilityDistribution->plotData(moment.utilityDistribution);
    plotWealthDistribution->plotData(moment.wealthDistribution);

    plotQ1Traded->plotDataAndPercentage(
                history.q1Traded, time, simulation.amounts[0]);
    plotQ2Traded->plotDataAndPercentage(
                history.q2Traded, time, simulation.amounts[1]);
    plotSumUtility->plotData(
                history.sumUtilities, time);
    plotNumSuccessfulTrades->plotDataAndPercentage(
                history.numSuccessful, time, simulation.numActors/2);
    plotWealthDeviation->plotData(
                history.wealthDeviation, time);
}

void MainWindow::updateTimeRange()
{
    size_t currentTimeUnit = simulation.history.time - 1;
    ui->sliderTime->setMaximum(currentTimeUnit);
    ui->sliderTime->setValue(currentTimeUnit);
    ui->spinBoxTime->setValue(currentTimeUnit);
}

void MainWindow::plotNextSituation()
{
    Simulation::EdgeworthSituation const& nextSituation = simulation.provideNextSituation();
    plotEdgeworth(ui->plotEdgeworthBox, nextSituation);
}

void MainWindow::updateProgress()
{
    ui->labelProgress->setText(QString::number(simulation.progress.getDone())
                               + "/"
                               + QString::number(simulation.progress.getNum()));
    ui->progressBarRound->setValue(simulation.progress.getDone());
}

void MainWindow::updateCaseInput()
{
    QString caseName;
    do {
        caseName = caseNameManager.provideNextCaseName();
    } while (simulationCases.find(caseName) != simulationCases.end());
    ui->lineEditCaseName->setText(caseName);
    setButtonColor(ui->pushButtonCaseColor, colorManager.provideNextColor());
}

QColor MainWindow::getButtonColor(QPushButton *button) const
{
    return button->palette().button().color();
}

void MainWindow::setButtonColor(QPushButton *button, QColor color)
{
    QString style("QPushButton { background-color: rgb(%1, %2, %3) }");
    button->setStyleSheet(style.arg(color.red()).arg(color.green()).arg(color.blue()));
}

void MainWindow::removeCase(QString caseName)
{
    //todo plot sync
    auto foundElement = simulationCases.find(caseName);
    simulationCases.erase(foundElement);
}

void MainWindow::removeSimulationCaseRows(std::function<bool (int)> pred)
{
    for (int i = 0; i < ui->tableWidgetCases->rowCount(); ++i) {
        if (pred(i)) {
            auto caseName = ui->tableWidgetCases->item(i, caseNameColumnIdx)->text();
            removeCase(caseName);
            ui->tableWidgetCases->removeRow(i);
            --i; //repeat this index in the loop
        }
    }
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

    setupSimulationByForm();

    ui->progressBarRound->setMaximum(simulation.progress.getNum());

    plotNextSituation();
    updateOverview();
    updateProgress();
    updateTimeRange();

    changeToTab(ui->tabWidget, ui->tabMainOverview);
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
    updateTimeRange();
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
        updateTimeRange();
    }
    plotNextSituation();
}

void MainWindow::on_sliderSpeed_valueChanged(int)
{
    timer->setInterval(calculateSpeedInterval());
}

void MainWindow::on_sliderTime_valueChanged(int value)
{
    //todo spare double load
    loadHistoryMoment(value);
}

void MainWindow::on_pushButtonRegenerateSeed_clicked()
{
    ui->lineEditSeed->setText(QString::number(globalUrng()));
}

QString appGroupKey = "application";
QString configVersionKey = "config_version";
QString currentConfigVersion = "1.0";

QString simulationGroupKey = "simulation";
QString q1SumKey = "q1_sum";
QString q2SumKey = "q2_sum";
QString numActorsKey = "num_actors";
QString randomSeedKey = "random_seed";

QString offerStrategyKey = "offer_strategy";
QString oppositeParetoValue = "opposite pareto";
QString randomParetoValue = "random pareto";
QString randomTriangleValue = "random triangle";

QString acceptanceStrategyKey = "acceptance_strategy";
QString alwaysValue = "always";
QString higherGainValue = "want higher gain";
QString higherProportionValue = "want higher proportion";

void MainWindow::on_actionSaveConfiguration_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save configuration", "", "(*ini).");
    if (fileName != "") {
        if (!fileName.endsWith(".ini")) {
            fileName += ".ini";
        }
        QSettings settings(fileName, QSettings::IniFormat);
        settings.beginGroup(appGroupKey);
        settings.setValue(configVersionKey, "1.0");
        settings.endGroup();

        settings.beginGroup(simulationGroupKey);
        settings.setValue(q1SumKey, ui->lineEditSumQ1->text());
        settings.setValue(q2SumKey, ui->lineEditSumQ2->text());
        settings.setValue(numActorsKey, ui->lineEditNumActors->text());
        settings.setValue(randomSeedKey, ui->lineEditSeed->text());

        QString offerStrategy = "";
        if (ui->radioButtonOppositePareto->isChecked()) {
            offerStrategy = oppositeParetoValue;
        } else if (ui->radioButtonRandomPareto->isChecked()) {
            offerStrategy = randomParetoValue;
        } else if (ui->radioButtonRandomTriangle->isChecked()) {
            offerStrategy = randomTriangleValue;
        }
        settings.setValue(offerStrategyKey, offerStrategy);

        QString acceptanceStrategy = "";
        if (ui->radioButtonWantAlways->isChecked()) {
            acceptanceStrategy = alwaysValue;
        } else if (ui->radioButtonWantHigherGain->isChecked()) {
            acceptanceStrategy = higherGainValue;
        } else if (ui->radioButtonWantHigherProportion->isChecked()) {
            acceptanceStrategy = higherProportionValue;
        }
        settings.setValue(acceptanceStrategyKey, acceptanceStrategy);
        settings.endGroup();
    }
}

void MainWindow::on_actionLoadConfiguration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load configuration", "", "(*ini).");
    if (fileName != "") {
        QSettings settings(fileName, QSettings::IniFormat);
        settings.beginGroup(simulationGroupKey);
        ui->lineEditSumQ1->setText(settings.value(q1SumKey).toString());
        ui->lineEditSumQ2->setText(settings.value(q2SumKey).toString());
        ui->lineEditNumActors->setText(settings.value(numActorsKey).toString());
        ui->lineEditSeed->setText(settings.value(randomSeedKey).toString());

        QString offerStrategy = settings.value(offerStrategyKey).toString();
        if (offerStrategy == oppositeParetoValue) {
            ui->radioButtonOppositePareto->setChecked(true);
        } else if (offerStrategy == randomParetoValue) {
            ui->radioButtonRandomPareto->setChecked(true);
        } else if (offerStrategy == randomTriangleValue) {
            ui->radioButtonRandomTriangle->setChecked(true);
        }

        QString acceptanceStrategy = settings.value(acceptanceStrategyKey).toString();
        if (acceptanceStrategy == alwaysValue) {
            ui->radioButtonWantAlways->setChecked(true);
        } else if (acceptanceStrategy == higherGainValue) {
            ui->radioButtonWantHigherGain->setChecked(true);
        } else if (acceptanceStrategy == higherProportionValue) {
            ui->radioButtonWantHigherProportion->setChecked(true);
        }
    }
}

void MainWindow::on_pushButtonClearHistory_clicked()
{
    removeSimulationCaseRows([](int){return true;}); //remove all lines
    colorManager.reset();
    caseNameManager.reset();
    updateCaseInput();
}

void MainWindow::on_pushButtonAddCurrentOutput_clicked()
{
    auto caseName = ui->lineEditCaseName->text();
    if (caseName == "" || simulationCases.find(caseName) != simulationCases.end()) {
        QMessageBox msgBox;
        msgBox.setText("You have to enter a non-empty unique name for the case");
        msgBox.exec();
    } else {
        auto& simulationCase = simulationCases[caseName];
        simulationCase.simulation = simulation;
        simulationCase.color = getButtonColor(ui->pushButtonCaseColor);
        simulationCase.isShown = true;

        auto const rowIdx = ui->tableWidgetCases->rowCount();
        ui->tableWidgetCases->insertRow(rowIdx);

        static auto disableItem = [](QTableWidgetItem* item){
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        };
        auto nameItem = new QTableWidgetItem(caseName);
        disableItem(nameItem);
        ui->tableWidgetCases->setItem(rowIdx, caseNameColumnIdx, nameItem);

        auto coloredItem = new QTableWidgetItem("");
        disableItem(coloredItem);
        coloredItem->setBackgroundColor(simulationCase.color);
        ui->tableWidgetCases->setItem(rowIdx, caseColorColumnIdx, coloredItem);

        auto checkItem = new QTableWidgetItem("");
        disableItem(checkItem);
        checkItem->setCheckState(simulationCase.isShown ? Qt::Checked : Qt::Unchecked);
        ui->tableWidgetCases->setItem(rowIdx, checkBoxColumnIdx, checkItem);

        updateCaseInput();

        //todo plot sync
    }
}

void MainWindow::on_tableWidgetCases_cellChanged(int row, int column)
{
    if (column == checkBoxColumnIdx) {
        auto const caseName = ui->tableWidgetCases->item(row, caseNameColumnIdx)->text();
        auto const checkItem = ui->tableWidgetCases->item(row, column);
        auto const isChecked = checkItem->checkState() == Qt::Checked;
        simulationCases[caseName].isShown = isChecked;
        //todo plot sync
    }
}

void MainWindow::on_pushButtonCaseColor_clicked()
{
    auto color = QColorDialog::getColor(getButtonColor(ui->pushButtonCaseColor));
    if (color.isValid()) {
        setButtonColor(ui->pushButtonCaseColor, color);
    }
}

void MainWindow::on_pushButtonDeleteSelectedOutput_clicked()
{
    removeSimulationCaseRows(
        [this](int rowIdx) {
            for (int j = 0; j < ui->tableWidgetCases->columnCount(); ++j) {
                if (ui->tableWidgetCases->item(rowIdx, j)->isSelected()) {
                    return true;
                }
            }
            return false;
        }
    );
}
