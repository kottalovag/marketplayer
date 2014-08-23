#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <ctime>
#include <random>
#include <algorithm>
#include <numeric>
#include <functional>

#include <iostream>

#include <unordered_map>
#include <vector>
#include <tuple>
#include <list>

#include <QVector>

#include "qcustomplot.h"

typedef std::mt19937 URNG;
URNG urng;

using std::tuple;
using std::make_tuple;

using std::cout;
using std::endl;

template<typename E>
//using vector = std::vector<E>;
using vector = QVector<E>;

struct Simulation;

struct IndexNumber
{
    unsigned int actual;
    IndexNumber() {
        actual = 0;
    }
    unsigned int operator()() {return actual++;}
};

typedef double Amount_t;

template<typename T>
struct DataPair
{
    vector<T> x, y;
    typename vector<T>::size_type size() const { return x.size(); }
    void push(T const& x, T const& y) {
        this->x.push_back(x);
        this->y.push_back(y);
    }
};

typedef DataPair<Amount_t> ResourceDataPair;

struct Utility
{
    double alfa1, alfa2;
    Amount_t compute(Amount_t q1, Amount_t q2) const {
        return pow(q1, alfa1)*pow(q2, alfa2);
    }
};

struct IndifferenceCurve
{
    Utility utility;
    Amount_t fixQ1, fixQ2;

    IndifferenceCurve(Utility utility, Amount_t q1, Amount_t q2)
        : utility(utility)
        , fixQ1(q1)
        , fixQ2(q2)
    {}

    ResourceDataPair getFixPoint() const {
        ResourceDataPair result;
        result.push(fixQ1, fixQ2);
        return result;
    }

    Amount_t getQ2(Amount_t q1) const {
        return fixQ2 * pow(fixQ1/q1, utility.alfa1/utility.alfa2);
    }
};

struct ResourceToleranceEquality
{
    bool operator()(Amount_t const& x, Amount_t const& y) const {
        return fabs(x-y) < std::numeric_limits<Amount_t>::epsilon();
    }
};

typedef std::unordered_map<Amount_t, bool, std::hash<Amount_t>, ResourceToleranceEquality> PinPointMap;

struct Simulation
{
    struct ActorConstRef {
        Amount_t const& q1;
        Amount_t const& q2;
        ActorConstRef(Simulation const& simulation, size_t const idx)
            : q1(simulation.mResources[0][idx])
            , q2(simulation.mResources[1][idx])
        {}
    };

    struct ActorRef {
        Amount_t& q1;
        Amount_t& q2;
        ActorRef(Simulation& simulation, size_t const idx)
            : q1(simulation.mResources[0][idx])
            , q2(simulation.mResources[1][idx])
        {}
    };

    struct EdgeworthSituation {
        ActorRef actor1, actor2;
        IndifferenceCurve curve1, curve2;
        Amount_t const q1Sum, q2Sum;
        EdgeworthSituation(Simulation& simulation, size_t const actor1Idx, size_t const actor2Idx)
            : actor1(simulation, actor1Idx)
            , actor2(simulation, actor2Idx)
            , curve1(simulation.mUtility, actor1.q1, actor1.q2)
            , curve2(simulation.mUtility, actor2.q1, actor2.q2)
            , q1Sum(actor1.q1 + actor2.q1)
            , q2Sum(actor1.q2 + actor2.q2)
        {
        }

        std::function<Amount_t(Amount_t)> getCurve1Function() const {
            return [this](double q1){ return curve1.getQ2(q1); };
        }

        std::function<Amount_t(Amount_t)> getCurve2Function() const {
            return [this](double q1){ return q2Sum - curve2.getQ2(q1Sum - q1); };
        }

        //implicit hack: linear contract curve
        std::function<Amount_t(Amount_t)> getParetoSetFunction() const {
            return [this](double q1){ return q1 * q2Sum/q1Sum; };
        }

        ResourceDataPair getFixPoint() const {
            return curve1.getFixPoint();
        }

        Amount_t calculateCurve1ParetoIntersection() const {
            return calculateParetoIntersection(curve1);
        }

        //implicit hack of linear contract curve
        Amount_t calculateCurve2ParetoIntersection() const {
            return q1Sum - calculateParetoIntersection(curve2);
        }

    private:
        //implicit hack of linear contract-curve
        Amount_t calculateParetoIntersection(IndifferenceCurve const& curve) const {
            double const alfa1 = curve.utility.alfa1;
            double const alfa2 = curve.utility.alfa2;
            return pow(q1Sum/q2Sum * curve.fixQ2 * pow(curve.fixQ1, alfa1/alfa2),
                       alfa2/(alfa1+alfa2));
        }
    };

    Utility mUtility;
    vector<vector<Amount_t>> mResources;
    vector<size_t> mPermutation;
    size_t mNumActors;
    vector<Amount_t> mAmounts;

    void setupPermutation() {
        mPermutation.resize(mNumActors);
        std::generate(mPermutation.begin(), mPermutation.end(), IndexNumber());
    }

    void shufflePermutation() {
        std::shuffle(mPermutation.begin(), mPermutation.end(), urng);
    }

    bool checkResources(size_t resourceIdx) {
        auto const& res = mResources[resourceIdx];
        Amount_t const sum = std::accumulate(res.begin(), res.end(), 0.0);
        cout << "asserted sum: " << sum << " actual sum: " << mAmounts[resourceIdx] << endl;
        //return sum <= mAmounts[resourceIdx];
        return fabs(sum - mAmounts[resourceIdx]) < std::numeric_limits<Amount_t>::epsilon() * mNumActors;
    }

    void printResources(size_t resourceIdx) {
        Amount_t sum = 0.0;
        foreach (auto res, mResources[resourceIdx]) {
            sum += res;
            cout << res << ",";
        }
        cout << endl;
        cout << "sum: " << sum << endl;
    }

    static void setupResources(vector<Amount_t>& resources, Amount_t const sumAmount, size_t const numActors) {
        resources.reserve(numActors);
        resources.resize(0);
        PinPointMap isPinPointUsed;
        vector<Amount_t>& pinPoints = resources; //alias for readability
        std::uniform_real_distribution<Amount_t> uniformDistribution(0.0, sumAmount);
        size_t repetitions = 0;
        while (pinPoints.size() < numActors) {
            Amount_t pinPoint = uniformDistribution(urng);
            if (!isPinPointUsed[pinPoint]) {
                isPinPointUsed[pinPoint] = true;
                pinPoints.push_back(pinPoint);
            } else ++repetitions;
        }
        cout << "random repetitions: " << repetitions << endl;
        std::sort(pinPoints.begin(), pinPoints.end());

        //resources and pinPoints is the same container
        //the alias was used for sake of clarity
        //we have to be careful about the order
        Amount_t const amountAtBorder = pinPoints[0] + (sumAmount - pinPoints.back());
        for (size_t actorIdx = numActors-1; actorIdx > 0; --actorIdx) {
            resources[actorIdx] = pinPoints[actorIdx] - pinPoints[actorIdx-1];
        }
        resources[0] = amountAtBorder;
    }

    bool setup(size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2) {
        if (numActors%2 != 0) return false;
        mAmounts.resize(0);
        mAmounts.push_back(amountQ1);
        mAmounts.push_back(amountQ2);
        mResources.resize(mAmounts.size());
        mNumActors = numActors;
        mUtility.alfa1 = alfa1;
        mUtility.alfa2 = alfa2;
        setupPermutation();
        for (size_t idx = 0; idx < mAmounts.size(); ++idx) {
            setupResources(mResources[idx], mAmounts[idx], numActors);
            //Q_ASSERT(checkResources(idx));
        }
        return true;
    }

    void nextRound() {
        shufflePermutation();
        for (size_t idx = 0; idx < mNumActors; idx += 2) {
            EdgeworthSituation situation(*this, mPermutation[idx], mPermutation[idx+1]);
        }
    }

    ResourceDataPair sampleFunction(std::function<double(double)> func, Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution) const {
        ResourceDataPair dataPair;
        for (auto x = rangeStart; x <= rangeFinish; x += resolution) {
            dataPair.push(x, func(x));
        }
        return dataPair;
    }

    void addCrossPoint(QCPGraph* graph, QColor color, ResourceDataPair const& point) const {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross));
        auto pen = graph->pen();
        pen.setColor(color);
        pen.setWidth(2);
        graph->setPen(pen);
        graph->setData(point.x, point.y);
    }

    void plotEdgeworth(QCustomPlot* plot, EdgeworthSituation const& situation) const {
        while (plot->graphCount() < 6) {
            plot->addGraph();
        }
        Amount_t const q1RangeStart = std::numeric_limits<Amount_t>::epsilon();
        Amount_t const q1Resolution = 0.1;

        plot->xAxis->setRange(0.0, situation.q1Sum);
        plot->yAxis->setRange(0.0, situation.q2Sum);

        auto graph1 = plot->graph(0);
        ResourceDataPair data1 = sampleFunction(situation.getCurve1Function(), q1RangeStart, situation.q1Sum, q1Resolution);
        //ResourceDataPair data1 = sampleFunction(situation.getCurve1Function(), situation.actor1.q1, situation.q1Sum, q1Resolution);
        /*auto graph1Brush = graph1->brush();
        graph1Brush.setStyle(Qt::SolidPattern);
        QColor fillColor = Qt::blue;
        fillColor.setAlpha(32);
        graph1Brush.setColor(fillColor);
        graph1->setBrush(graph1Brush);
        graph1->setChannelFillGraph(plot->graph(1));*/
        graph1->setData(data1.x, data1.y);

        ResourceDataPair data2 = sampleFunction(situation.getCurve2Function(), q1RangeStart, situation.q1Sum, q1Resolution);
        plot->graph(1)->setData(data2.x, data2.y);

        auto fixPoint = situation.getFixPoint();
        addCrossPoint(plot->graph(2), Qt::red, fixPoint);

        auto paretoSet = plot->graph(3);
        paretoSet->setPen(QPen(Qt::green));
        ResourceDataPair paretoData = sampleFunction(situation.getParetoSetFunction(),
                                                     q1RangeStart, situation.q1Sum, q1Resolution);
        paretoSet->setData(paretoData.x, paretoData.y);

        ResourceDataPair paretoPoint1Data;
        Amount_t const p1_q1 = situation.calculateCurve1ParetoIntersection();
        Amount_t const p1_q2 = situation.getCurve1Function()(p1_q1);
        cout << "Pareto1: " << p1_q1 << " ; " << p1_q2 << endl;
        paretoPoint1Data.push(p1_q1, p1_q2);
        addCrossPoint(plot->graph(4), Qt::red, paretoPoint1Data);

        ResourceDataPair paretoPoint2Data;
        Amount_t const p2_q1 = situation.calculateCurve2ParetoIntersection();
        Amount_t const p2_q2 = situation.getCurve2Function()(p2_q1);
        cout << "Pareto2: " << p2_q1 << " ; " << p2_q2 << endl;
        paretoPoint2Data.push(p2_q1, p2_q2);
        addCrossPoint(plot->graph(5), Qt::red, paretoPoint2Data);
    }
};

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
