#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "model.h"
#include "qcustomplot.h"
#include "plotutils.h"
#include "strategymapper.h"

#include <iostream>
#include <time.h>
#include <QFileDialog>
#include <QMap>

using std::cout;
using std::endl;

std::function<void(Position const&)> debugShowPoint;

//Ugly, yes. I know. Anyway, the user will have to type in characters, so why not? :)
const QString MainWindow::mainSimulationID = "";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    globalUrng.seed(time(0));
    ui->setupUi(this);

    appWaitingForSimulationLoaded.reset(new AppWaitingForSimulationLoaded(this));
    appInSimulationMode.reset(new AppInSimulationMode(this));
    appInComparisonMode.reset(new AppInComparisonMode(this));

    setupControlsAndUIStartup();

    caseManager.reset(new CaseManager(
                          plotQ1Traded.get(),
                          plotQ2Traded.get(),
                          plotSumUtility.get(),
                          plotNumSuccessfulTrades.get(),
                          plotWealthDeviation.get(),
                          plotQ1Distribution.get(),
                          plotQ2Distribution.get(),
                          plotUtilityDistribution.get(),
                          plotWealthDistribution.get()
                          )
                      );
    caseManager->addExternalCase(mainSimulationID, simulation, Qt::blue, true);

    strategyMap[oppositeParetoValue] = ui->radioButtonOppositePareto;
    strategyMap[randomParetoValue] = ui->radioButtonRandomPareto;
    strategyMap[randomTriangleValue] = ui->radioButtonRandomTriangle;

    strategyMap[alwaysValue] = ui->radioButtonWantAlways;
    strategyMap[higherGainValue] = ui->radioButtonWantHigherGain;
    strategyMap[higherProportionValue] = ui->radioButtonWantHigherProportion;

    applyUIToApplicationStarted();

    debugShowPoint = [this](Position p){
        auto debugGraph = ui->plotEdgeworthBox->graph(5);
        debugGraph->addData(p.q1, p.q2);
    };

    setState(appWaitingForSimulationLoaded.get());
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

void MainWindow::setState(AppState *nextState)
{
    currentState = nextState;
    currentState->enter();
}

void MainWindow::setupControlsAndUIStartup()
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
    ui->lineEditAlfa1->setText(QString::number(defaultAlfa1));
    ui->lineEditAlfa1->setValidator(alfaValidator);
    ui->lineEditAlfa1->setEnabled(false); //TODO resolve
    ui->lineEditAlfa2->setText(QString::number(defaultAlfa2));
    ui->lineEditAlfa2->setValidator(alfaValidator);
    ui->lineEditAlfa2->setEnabled(false); //TODO resolve

    ui->lineEditSeed->setValidator(new QIntValidator(0,std::numeric_limits<URNG::result_type>::max(), this));
    on_pushButtonRegenerateSeed_clicked();

    ui->radioButtonOppositePareto->setEnabled(true);
    ui->radioButtonWantAlways->setEnabled(true);

    connect(ui->sliderTime, SIGNAL(rangeChanged(int,int)),
            this, SLOT(onSliderTimeRangeChanged(int,int)));

    connect(ui->buttonGroupOverviewMode, SIGNAL(buttonClicked(int)),
            this, SLOT(onButtonGroupOverviewMode_buttonClicked(int)));

    ui->toolButtonApply->setDefaultAction(ui->actionApply);
    ui->toolButtonRevertChanges->setDefaultAction(ui->actionRevertChanges);
    ui->toolButtonNextRound->setDefaultAction(ui->actionNextRound);
    ui->toolButtonStart->setDefaultAction(ui->actionStart);
    ui->toolButtonPause->setDefaultAction(ui->actionPause);
    ui->toolButtonNextTrade->setDefaultAction(ui->actionNextTrade);

    ui->sliderTime->setValue(0);

    setupSpeedControls();

    setupEdgeworthBox();

    plotQ1Traded.reset(new DataTimeRatioPlot(
        ui->plotQ1Traded, ui->labelQ1Traded, "Q1 traded", "Q1 traded"));
    plotQ2Traded.reset(new DataTimeRatioPlot(
        ui->plotQ2Traded, ui->labelQ2Traded, "Q2 traded", "Q2 traded"));
    plotSumUtility.reset(new DataTimePlot(
        ui->plotSumUtility, ui->labelSumUtilities, "Sum of utilities", "Sum of utilities"));
    plotNumSuccessfulTrades.reset(new DataTimeRatioPlot(
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
}

void MainWindow::unmarkParameterControls()
{
    for (auto item : {
         ui->lineEditNumActors,
         ui->lineEditSumQ1,
         ui->lineEditSumQ2,
         ui->lineEditAlfa1,
         ui->lineEditAlfa2,
         ui->lineEditSeed}
         )
    {
        markLineEditChanged(item, false);
    }
    for (auto item : {
         ui->groupBoxOfferStrategy,
         ui->groupBoxAcceptanceStrategy}
         )
    {
        markGroupBoxChanged(item, false);
    }
}

int MainWindow::calculateSpeedInterval() const {
    auto const maxValue = ui->sliderSpeed->maximum();
    auto const maxTime = 1000;
    auto const value = ui->sliderSpeed->value();
    double ratio = (double)value/(double)maxValue;
    return maxTime * ratio;
}

void MainWindow::applyUIToApplicationStarted()
{
    changeToTab(ui->tabWidget, ui->tabSetup);

    updateCaseInput();

    ui->groupBoxHistory->setEnabled(false);
    ui->actionSaveConfiguration->setEnabled(false);

    ui->sliderTime->setMinimum(0);
    ui->sliderTime->setMaximum(0);

    //todo plot sync
    plotQ1Traded->clearData();
    plotQ2Traded->clearData();
    plotSumUtility->clearData();
    plotWealthDeviation->clearData();
    plotNumSuccessfulTrades->clearData();

    plotQ1Distribution->clearData();
    plotQ2Distribution->clearData();
    plotUtilityDistribution->clearData();
    plotWealthDistribution->clearData();

    clearPlotData(ui->plotEdgeworthBox);

    ui->actionSaveEdgeworthDiagram->setEnabled(false);

    ui->actionNextRound->setEnabled(false);
    ui->actionStart->setEnabled(false);
    ui->actionPause->setEnabled(false);
    ui->actionNextTrade->setEnabled(false);
}

void MainWindow::applyUIToSimulationSetup()
{
    ui->progressBarRound->setMaximum(simulation.progress.getNum());

    plotNextSituation();
    loadHistoryMoment(simulation.history.time - 1);
    updateProgress();
    updateTimeRangeBySimulation();

    changeToTab(ui->tabWidget, ui->tabMainOverview);
    ui->groupBoxHistory->setEnabled(true);
    ui->actionSaveConfiguration->setEnabled(true);
    unmarkParameterControls();
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

void MainWindow::plotEdgeworth(QCustomPlot* plot, EdgeworthSituation const& situation) {
    clearPlotData(plot);

    Amount_t const q1RangeStart = std::numeric_limits<Amount_t>::epsilon();
    Amount_t const q1Resolution = situation.q1Sum / 4000; //todo generalize

    plot->xAxis->setRange(0.0, situation.q1Sum);
    plot->yAxis->setRange(0.0, situation.q2Sum);

    auto curve1Graph = plot->graph(0);
    ResourceDataPair data1 = sampleFunction(
                situation.getCurve1Function(), q1RangeStart, situation.q1Sum, q1Resolution);
    curve1Graph->setData(data1.x, data1.y);

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

void MainWindow::loadHistoryMoment(int time)
{
    caseManager->updatePlotsAt(time);
}

void MainWindow::updateTimeRange(size_t time)
{
    ui->sliderTime->setMaximum(time);
    ui->sliderTime->setValue(time);
    ui->spinBoxTime->setValue(time);
}

void MainWindow::updateTimeRangeBySimulation()
{
    updateTimeRange(simulation.history.time - 1);
}

void MainWindow::plotNextSituation()
{
    EdgeworthSituation const& nextSituation = simulation.provideNextSituation();
    plotEdgeworth(ui->plotEdgeworthBox, nextSituation);
}

bool MainWindow::trySetupSimulationByForm()
{
    bool success = simulation.setup(ui->lineEditSeed->text().toUInt(),
                     ui->lineEditNumActors->text().toInt(),
                     ui->lineEditSumQ1->text().toDouble(),
                     ui->lineEditSumQ2->text().toDouble(),
                     ui->lineEditAlfa1->text().toDouble(),
                     ui->lineEditAlfa2->text().toDouble());

    if (success) {
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
    return success;
}

void MainWindow::setupSimulationByHistory(const AbstractSimulationCase &simulationCase)
{
    simulation = simulationCase.getSimulation();
}

void MainWindow::updateParameterControlsFromSimulation(const Simulation &simulation)
{
    ui->lineEditNumActors->setText(QString::number(simulation.numActors));
    ui->lineEditSumQ1->setText(QString::number(simulation.amounts[0]));
    ui->lineEditSumQ2->setText(QString::number(simulation.amounts[1]));
    ui->lineEditAlfa1->setText(QString::number(simulation.utility.alfa1));
    ui->lineEditAlfa2->setText(QString::number(simulation.utility.alfa2));
    ui->lineEditSeed->setText(QString::number(simulation.seed));

    OfferStrategyNameVisitor ov;
    if (simulation.offerStrategy.get()) {
        auto offerStrategyName = ov.getStrategyDescription(*simulation.offerStrategy);
        strategyMap[offerStrategyName]->setChecked(true);
    }

    AcceptanceStrategyNameVisitor av;
    if (simulation.acceptanceStrategy.get()) {
        auto acceptanceStrategyName = av.getStrategyDescription(*simulation.acceptanceStrategy);
        strategyMap[acceptanceStrategyName]->setChecked(true);
    }
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
    } while (caseManager->contains(caseName));
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

void MainWindow::addCaseRow(bool visible)
{
    auto caseName = ui->lineEditCaseName->text();
    if (caseName == "" || caseName == mainSimulationID || caseManager->contains(caseName)) {
        QMessageBox msgBox;
        msgBox.setText("You have to enter a non-empty unique name for the case");
        msgBox.exec();
    } else {
        auto const color = getButtonColor(ui->pushButtonCaseColor);
        caseManager->addHeavyCase(caseName, simulation, color, visible);

        auto const rowIdx = getNumCaseRows();
        ui->tableWidgetCases->insertRow(rowIdx);

        static auto disableItem = [](QTableWidgetItem* item){
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        };
        auto nameItem = new QTableWidgetItem(caseName);
        disableItem(nameItem);
        ui->tableWidgetCases->setItem(rowIdx, caseNameColumnIdx, nameItem);

        auto coloredItem = new QTableWidgetItem("");
        disableItem(coloredItem);
        coloredItem->setBackgroundColor(color);
        ui->tableWidgetCases->setItem(rowIdx, caseColorColumnIdx, coloredItem);

        auto checkItem = new QTableWidgetItem("");
        disableItem(checkItem);
        checkItem->setCheckState(Qt::Checked); //checked by default
        ui->tableWidgetCases->setItem(rowIdx, checkBoxColumnIdx, checkItem);

        updateCaseInput();
        loadHistoryMoment(ui->sliderTime->value());
    }
}

void MainWindow::removeCaseRows(std::function<bool (int)> pred)
{
    for (size_t i = 0; i < getNumCaseRows(); ++i) {
        if (pred(i)) {
            caseManager->removeCase(getCaseNameFromRow(i));
            ui->tableWidgetCases->removeRow(i);
            --i; //repeat this index in the loop
        }
    }
}

size_t MainWindow::getNumCaseRows() const
{
    return ui->tableWidgetCases->rowCount();
}

QString MainWindow::getCaseNameFromRow(size_t rowIdx) const
{
    return ui->tableWidgetCases->item(rowIdx, caseNameColumnIdx)->text();
}

bool MainWindow::isCaseRowSelected(size_t rowIdx) const
{
    for (int j = 0; j < ui->tableWidgetCases->columnCount(); ++j) {
        if (ui->tableWidgetCases->item(rowIdx, j)->isSelected()) {
            return true;
        }
    }
    return false;
}

bool MainWindow::isCaseRowChecked(size_t rowIdx) const
{
    auto const checkItem = ui->tableWidgetCases->item(rowIdx, checkBoxColumnIdx);
    return checkItem->checkState() == Qt::Checked;
}

int MainWindow::getFirstSelectedSimulationCaseRow() const
{
    size_t idx = 0;
    bool found = false;
    while (!found && idx < getNumCaseRows()) {
        found = isCaseRowSelected(idx);
        if (!found) ++idx;
    }
    if (found) {
        return idx;
    } else {
        return -1;
    }
}

void MainWindow::setupShownHistoryCases()
{
    for (size_t i = 0; i < getNumCaseRows(); ++i) {
        caseManager->setVisibility(getCaseNameFromRow(i), isCaseRowChecked(i));
    }
}

void MainWindow::markLineEditChanged(QLineEdit *lineEdit, bool marked)
{
    QString style;
    if (marked) {
        style = "QLineEdit { background-color: yellow }";
    }
    lineEdit->setStyleSheet(style);
}

void MainWindow::markGroupBoxChanged(QGroupBox *groupBox, bool marked)
{
    QString style;
    if (marked) {
        style = "QGroupBox { background-color: yellow }";
    }
    groupBox->setStyleSheet(style);
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

void MainWindow::on_buttonGroupOfferStrategy_buttonClicked(int)
{
    markGroupBoxChanged(ui->groupBoxOfferStrategy, true);
}

void MainWindow::on_buttonGroupAcceptanceStrategy_buttonClicked(int)
{
    markGroupBoxChanged(ui->groupBoxAcceptanceStrategy, true);
}

void MainWindow::onButtonGroupOverviewMode_buttonClicked(int)
{
    if (ui->radioButtonSimulation->isChecked()) {
        currentState->simulationModeSelected();
    } else {
        currentState->comparisonModeSelected();
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
    currentState->beforeSimulationSetup();
    bool success = trySetupSimulationByForm();
    if (success) {
        applyUIToSimulationSetup();
        currentState->simulationSetupOccured();
    } else {
        QMessageBox msgBox;
        msgBox.setText("The simulation could not be setup using these parameters!");
        msgBox.exec();
    }
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
    updateTimeRangeBySimulation();
    updateProgress();
    loadHistoryMoment(simulation.history.time - 1);
}

void MainWindow::on_actionStart_triggered()
{
    timer->start();

    ui->actionStart->setEnabled(false);
    ui->actionPause->setEnabled(true);
    ui->actionNextTrade->setEnabled(false);
    ui->actionSaveEdgeworthDiagram->setEnabled(false);
}

void MainWindow::on_actionPause_triggered()
{
    timer->stop();

    ui->actionNextRound->setEnabled(true);
    ui->actionStart->setEnabled(true);
    ui->actionPause->setEnabled(false);
    ui->actionNextTrade->setEnabled(true);
    ui->actionSaveEdgeworthDiagram->setEnabled(true);
}

void MainWindow::on_actionNextTrade_triggered()
{
    bool const isFinished = simulation.performNextTrade();
    updateProgress();
    if (isFinished) {
        updateTimeRangeBySimulation();
    }
    plotNextSituation();
}

void MainWindow::on_sliderSpeed_valueChanged(int)
{
    timer->setInterval(calculateSpeedInterval());
}

void MainWindow::on_sliderTime_valueChanged(int value)
{
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
QString acceptanceStrategyKey = "acceptance_strategy";

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
        settings.setValue(q1SumKey, QString::number(simulation.amounts[0]));
        settings.setValue(q2SumKey, QString::number(simulation.amounts[1]));
        settings.setValue(numActorsKey, QString::number(simulation.numActors));
        settings.setValue(randomSeedKey, QString::number(simulation.seed));
        OfferStrategyNameVisitor ov;
        settings.setValue(offerStrategyKey, ov.getStrategyDescription(*simulation.offerStrategy));
        AcceptanceStrategyNameVisitor av;
        settings.setValue(acceptanceStrategyKey, av.getStrategyDescription(*simulation.acceptanceStrategy));
        settings.endGroup();
    }
}

void MainWindow::on_actionLoadConfiguration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load configuration", "", "(*ini).");
    if (fileName != "") {
        currentState->beforeSimulationSetup();

        QSettings settings(fileName, QSettings::IniFormat);
        settings.beginGroup(simulationGroupKey);

        unsigned amountQ1 = settings.value(q1SumKey).toUInt();
        unsigned amountQ2 = settings.value(q2SumKey).toUInt();
        size_t numActors = settings.value(numActorsKey).toUInt();
        URNG::result_type seed = settings.value(randomSeedKey).toUInt();
        double alfa1 = defaultAlfa1;
        double alfa2 = defaultAlfa2;

        QString offerStrategy = settings.value(offerStrategyKey).toString();
        QString acceptanceStrategy = settings.value(acceptanceStrategyKey).toString();
        settings.endGroup();

        bool success = simulation.setup(seed, numActors, amountQ1, amountQ2, alfa1, alfa2);
        if (success) {
            simulation.offerStrategy = createOfferStrategy(offerStrategy);
            simulation.acceptanceStrategy = createAcceptanceStrategy(acceptanceStrategy);
            updateParameterControlsFromSimulation(simulation);
            applyUIToSimulationSetup();
            currentState->simulationSetupOccured();
        } else {
            QMessageBox msgBox;
            msgBox.setText("The simulation could not be setup using this configuration file!");
            msgBox.exec();
        }
    }
}

void MainWindow::on_pushButtonClearHistory_clicked()
{
    removeCaseRows([](int){return true;}); //remove all lines
    colorManager.reset();
    caseNameManager.reset();
    updateCaseInput();
}

void MainWindow::on_pushButtonAddCurrentOutput_clicked()
{
    currentState->handleCaseAddition();
}

void MainWindow::on_tableWidgetCases_cellChanged(int row, int column)
{
    if (column == checkBoxColumnIdx) {
        currentState->handleCaseRowVisibility(row);
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
    removeCaseRows(
                [this](int idx) { return isCaseRowSelected(idx); }
    );
}

void MainWindow::on_pushButtonLoadSelectedOutput_clicked()
{
    int idx = getFirstSelectedSimulationCaseRow();
    if (idx > -1) {
        currentState->beforeSimulationSetup();
        setupSimulationByHistory(caseManager->getSimulationCase(getCaseNameFromRow(idx)));
        updateParameterControlsFromSimulation(simulation);
        applyUIToSimulationSetup();
        currentState->simulationSetupOccured();
    }
}

//todo: mark only if really different from model
void MainWindow::on_lineEditNumActors_textChanged(const QString& )
{
    markLineEditChanged(ui->lineEditNumActors, true);
}

void MainWindow::on_lineEditSumQ1_textChanged(const QString&)
{
    markLineEditChanged(ui->lineEditSumQ1, true);
}

void MainWindow::on_lineEditSumQ2_textChanged(const QString&)
{
    markLineEditChanged(ui->lineEditSumQ2, true);
}

void MainWindow::on_lineEditAlfa1_textChanged(const QString&)
{
    markLineEditChanged(ui->lineEditAlfa1, true);
}

void MainWindow::on_lineEditAlfa2_textChanged(const QString&)
{
    markLineEditChanged(ui->lineEditAlfa2, true);
}

void MainWindow::on_lineEditSeed_textChanged(const QString&)
{
    markLineEditChanged(ui->lineEditSeed, true);
}

void MainWindow::on_tableWidgetCases_itemSelectionChanged()
{
    int idx = getFirstSelectedSimulationCaseRow();
    if (idx > -1) {
        auto const& simulationCase = caseManager->getSimulationCase(getCaseNameFromRow(idx));
        updateParameterControlsFromSimulation(simulationCase.getSimulation());
    }
}

void MainWindow::on_actionRevertChanges_triggered()
{
    updateParameterControlsFromSimulation(simulation);
    unmarkParameterControls();
}
