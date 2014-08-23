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
#include "model.h"

URNG urng;

using std::make_tuple;

using std::cout;
using std::endl;

template<typename E>
//using vector = std::vector<E>;
using vector = QVector<E>;

template<typename T>
void DataPair<T>::push(const T& x, const T& y) {
    this->x.push_back(x);
    this->y.push_back(y);
}


Amount_t Utility::compute(Amount_t q1, Amount_t q2) const {
    return pow(q1, alfa1)*pow(q2, alfa2);
}


IndifferenceCurve::IndifferenceCurve(Utility utility, Amount_t q1, Amount_t q2)
    : utility(utility)
    , fixQ1(q1)
    , fixQ2(q2)
{}

ResourceDataPair IndifferenceCurve::getFixPoint() const {
    ResourceDataPair result;
    result.push(fixQ1, fixQ2);
    return result;
}

Amount_t IndifferenceCurve::getQ2(Amount_t q1) const {
    return fixQ2 * pow(fixQ1/q1, utility.alfa1/utility.alfa2);
}


bool ResourceToleranceEquality::operator()(const Amount_t& x, const Amount_t& y) const {
    return fabs(x-y) < std::numeric_limits<Amount_t>::epsilon();
}


Simulation::EdgeworthSituation::EdgeworthSituation(Simulation& simulation, const size_t actor1Idx, const size_t actor2Idx)
    : actor1(simulation, actor1Idx)
    , actor2(simulation, actor2Idx)
    , curve1(simulation.utility, actor1.q1, actor1.q2)
    , curve2(simulation.utility, actor2.q1, actor2.q2)
    , q1Sum(actor1.q1 + actor2.q1)
    , q2Sum(actor1.q2 + actor2.q2)
{
}

std::function<Amount_t (Amount_t)> Simulation::EdgeworthSituation::getCurve1Function() const {
    return [this](double q1){ return curve1.getQ2(q1); };
}

std::function<Amount_t (Amount_t)> Simulation::EdgeworthSituation::getCurve2Function() const {
    return [this](double q1){ return q2Sum - curve2.getQ2(q1Sum - q1); };
}

//implicit hack: linear contract curve
std::function<Amount_t (Amount_t)> Simulation::EdgeworthSituation::getParetoSetFunction() const {
    return [this](double q1){ return q1 * q2Sum/q1Sum; };
}

Amount_t Simulation::EdgeworthSituation::calculateCurve1ParetoIntersection() const {
    return calculateParetoIntersection(curve1);
}

//implicit hack of linear contract curve
Amount_t Simulation::EdgeworthSituation::calculateCurve2ParetoIntersection() const {
    return q1Sum - calculateParetoIntersection(curve2);
}

//implicit hack of linear contract-curve
Amount_t Simulation::EdgeworthSituation::calculateParetoIntersection(const IndifferenceCurve& curve) const {
    double const alfa1 = curve.utility.alfa1;
    double const alfa2 = curve.utility.alfa2;
    return pow(q1Sum/q2Sum * curve.fixQ2 * pow(curve.fixQ1, alfa1/alfa2),
               alfa2/(alfa1+alfa2));
}

void Simulation::setupPermutation() {
    permutation.resize(numActors);
    std::generate(permutation.begin(), permutation.end(), IndexNumber());
}

void Simulation::shufflePermutation() {
    std::shuffle(permutation.begin(), permutation.end(), urng);
}

bool Simulation::checkResources(size_t resourceIdx) {
    auto const& res = resources[resourceIdx];
    Amount_t const sum = std::accumulate(res.begin(), res.end(), 0.0);
    cout << "asserted sum: " << sum << " actual sum: " << amounts[resourceIdx] << endl;
    //return sum <= mAmounts[resourceIdx];
    return fabs(sum - amounts[resourceIdx]) < std::numeric_limits<Amount_t>::epsilon() * numActors;
}

void Simulation::printResources(size_t resourceIdx) {
    Amount_t sum = 0.0;
    foreach (auto res, resources[resourceIdx]) {
        sum += res;
        cout << res << ",";
    }
    cout << endl;
    cout << "sum: " << sum << endl;
}

void Simulation::setupResources(vector<Amount_t>& resources, const Amount_t sumAmount, const size_t numActors) {
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

bool Simulation::setup(size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2) {
    if (numActors%2 != 0) return false;
    amounts.resize(0);
    amounts.push_back(amountQ1);
    amounts.push_back(amountQ2);
    resources.resize(amounts.size());
    numActors = numActors;
    utility.alfa1 = alfa1;
    utility.alfa2 = alfa2;
    setupPermutation();
    for (size_t idx = 0; idx < amounts.size(); ++idx) {
        setupResources(resources[idx], amounts[idx], numActors);
        //Q_ASSERT(checkResources(idx));
    }
    return true;
}

void Simulation::nextRound() {
    shufflePermutation();
    for (size_t idx = 0; idx < numActors; idx += 2) {
        EdgeworthSituation situation(*this, permutation[idx], permutation[idx+1]);
    }
}

ResourceDataPair sampleFunction(std::function<double (double)> func, Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution){
    ResourceDataPair dataPair;
    for (auto x = rangeStart; x <= rangeFinish; x += resolution) {
        dataPair.push(x, func(x));
    }
    return dataPair;
}

void Simulation::addCrossPoint(QCPGraph* graph, QColor color, const ResourceDataPair& point) const {
    graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross));
    auto pen = graph->pen();
    pen.setColor(color);
    pen.setWidth(2);
    graph->setPen(pen);
    graph->setData(point.x, point.y);
}

void Simulation::plotEdgeworth(QCustomPlot* plot, const Simulation::EdgeworthSituation& situation) const {
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
