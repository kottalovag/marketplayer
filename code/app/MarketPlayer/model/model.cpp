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


Amount_t Utility::compute(Amount_t q1, Amount_t q2) const {
    return pow(q1, alfa1)*pow(q2, alfa2);
}

IndifferenceCurve::IndifferenceCurve(Utility utility, Amount_t q1, Amount_t q2)
    : utility(utility)
    , fixP{q1, q2}
{}

Amount_t IndifferenceCurve::getQ2(Amount_t q1) const {
    return fixP.q2 * pow(fixP.q1/q1, utility.alfa1/utility.alfa2);
}

bool ResourceToleranceEquality::operator()(const Amount_t& x, const Amount_t& y) const {
    return fabs(x-y) < std::numeric_limits<Amount_t>::epsilon();
}

Simulation::EdgeworthSituation::EdgeworthSituation(const Simulation& simulation, const size_t actor1Idx, const size_t actor2Idx,
        AbstractTradeStrategy& tradeStrategy)
    : actor1(simulation, actor1Idx)
    , actor2(simulation, actor2Idx)
    , curve1(simulation.utility, actor1.q1, actor1.q2)
    , curve2(simulation.utility, actor2.q1, actor2.q2)
    , q1Sum(actor1.q1 + actor2.q1)
    , q2Sum(actor1.q2 + actor2.q2)
    , result(tradeStrategy.propose(*this))
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

//implicit hack of linear contract-curve
Amount_t Simulation::EdgeworthSituation::calculateParetoIntersectionQ1(const IndifferenceCurve& curve) const {
    double const alfa1 = curve.utility.alfa1;
    double const alfa2 = curve.utility.alfa2;
    return pow(q1Sum/q2Sum * curve.fixP.q2 * pow(curve.fixP.q1, alfa1/alfa2),
               alfa2/(alfa1+alfa2));
}

Position Simulation::EdgeworthSituation::calculateCurve1ParetoIntersection() const {
    Amount_t const q1 = calculateParetoIntersectionQ1(curve1);
    Amount_t const q2 = getCurve1Function()(q1);
    return Position{q1, q2};
}

//implicit hack of linear contract curve
Position Simulation::EdgeworthSituation::calculateCurve2ParetoIntersection() const {
    Amount_t const q1 = q1Sum - calculateParetoIntersectionQ1(curve2);
    Amount_t const q2 = getCurve2Function()(q1);
    return Position{q1, q2};
}

void Simulation::Progress::setup(size_t numActors) {
    permutation.resize(numActors);
    std::generate(permutation.begin(), permutation.end(), IndexNumber());
    shufflePermutation();
    actIdx = 0;
    restarted = false;
}

tuple<size_t, size_t> Simulation::Progress::getCurrentPair() const
{
    return make_tuple(permutation[actIdx], permutation[actIdx+1]);
}

bool Simulation::Progress::advance()
{
    restarted = false;
    actIdx += 2;
    bool const finished = isFinished();
    if (finished) {
        shufflePermutation();
        actIdx = 0;
        restarted = true;
    }
    return finished;
}

void Simulation::Progress::shufflePermutation() {
    std::shuffle(permutation.begin(), permutation.end(), urng);
}

bool Simulation::Progress::isFinished() const
{
    return actIdx == permutation.size();
}

void Simulation::setupResources(vector<Amount_t>& resources, const Amount_t sumAmount, const size_t numActors) {
    resources.resize(numActors);
    std::generate_n(resources.begin(), numActors, [](){
        std::uniform_real_distribution<Amount_t> uniformDistribution(0.0, 1.0);
        return uniformDistribution(urng);
    });
    Amount_t const sumRandom = std::accumulate(resources.begin(), resources.end(), 0.0);
    double const ratio = sumAmount/sumRandom;
    for (auto& element : resources) {
        element *= ratio;
    }
}

bool Simulation::setup(size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2) {
    if (numActors%2 != 0) return false;
    history.time = 0; //todo setup
    amounts.resize(0);
    amounts.push_back(amountQ1);
    amounts.push_back(amountQ2);
    resources.resize(amounts.size());
    this->numActors = numActors;
    utility.alfa1 = alfa1;
    utility.alfa2 = alfa2;
    progress.setup(numActors);
    for (size_t idx = 0; idx < amounts.size(); ++idx) {
        setupResources(resources[idx], amounts[idx], numActors);
    }
    return true;
}

Simulation::EdgeworthSituation Simulation::getNextSituation() const
{
    size_t actor1Idx, actor2Idx;
    std::tie(actor1Idx, actor2Idx) = progress.getCurrentPair();
    return EdgeworthSituation(*this, actor1Idx, actor2Idx, *tradeStrategy);
}

bool Simulation::performNextTrade()
{
    size_t actor1Idx, actor2Idx;
    std::tie(actor1Idx, actor2Idx) = progress.getCurrentPair();
    auto situation = getNextSituation();
    ActorRef actor1(*this, actor1Idx);
    ActorRef actor2(*this, actor2Idx);
    tradeStrategy->trade(situation, actor1, actor2);

    bool const progressFinished = progress.advance();
    if (progressFinished) {
        history.roundFinished();
    }
    return progressFinished;
}

void Simulation::performNextRound()
{
    while (!performNextTrade());
}

ResourceDataPair sampleFunction(std::function<double (double)> func, Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution){
    ResourceDataPair dataPair;
    for (auto x = rangeStart; x <= rangeFinish; x += resolution) {
        dataPair.push(x, func(x));
    }
    return dataPair;
}

vector<Amount_t> Simulation::computeUtilities() const {
    vector<Amount_t> utilities;
    for (size_t actorIdx = 0; actorIdx < numActors; ++actorIdx) {
        ActorConstRef actor(*this, actorIdx);
        utilities.push_back(utility.compute(actor.q1, actor.q2));
    }
    return utilities;
}

Distribution::Distribution(const vector<Amount_t>& subject, Amount_t resolution)
    : subject(subject)
    , resolution(resolution)
    , max(*std::max_element(subject.begin(), subject.end()))
    , numBuckets(static_cast<size_t>(ceil(max/resolution)))
{
    data.resize(numBuckets);

    //optimization possibility:
    Amount_t currentBucket = resolution/2;
    for (size_t idx = 0; idx < numBuckets; ++idx, currentBucket += resolution) {
        data.x[idx] = currentBucket;
    }

    for (auto const& amount : subject) {
        auto bucketIdx = static_cast<ResourceDataPair::size_type>(floor(amount/resolution));
        data.y[bucketIdx] += 1;
    }
}

Position OppositeParetoTradeStrategy::propose(Simulation::EdgeworthSituation& situation) const
{
    Position const p2 = situation.calculateCurve2ParetoIntersection();
    //debugShowPoint(p2);
    return p2;
}

Position RandomParetoTradeStrategy::propose(Simulation::EdgeworthSituation& situation) const
{
    Position const p2 = situation.calculateCurve2ParetoIntersection();
    Position const p1 = situation.calculateCurve1ParetoIntersection();
    double const factor = std::uniform_real_distribution<double>(0.0, 1.0)(urng);
    auto const result = p1 + (p2 - p1) * factor;
    //debugShowPoint(result);
    return result;
}

Position RandomTriangleTradeStrategy::propose(Simulation::EdgeworthSituation& situation) const
{
    Position const p0 = situation.getFixPoint();
    Position const p1 = situation.calculateCurve1ParetoIntersection();
    Position const p2 = situation.calculateCurve2ParetoIntersection();

    std::uniform_real_distribution<double> udist(0.0, 1.0);

    //we pick a point as if in a paralelogram for sake of uniformity
    auto const v01 = (p1 - p0);
    auto const v02 = (p2 - p0);
    double const factor1 = udist(urng);
    double const factor2 = udist(urng);
    Position px = p0 + v01*factor1 + v02*factor2;

    //if the point falls in the wrong half
    //  then we do point reflection around p1-p2 side's midde point
    //  to end up in the desired half
    if (!isPointInTriangle(p0, p1, p2, px)) {
        auto origo = p1 + (p2 - p1) * 0.5;
        px = px + (origo - px) * 2.0;
    }
    //debugShowPoint(px);
    return px;
}

bool isPointInTriangle(const Position& p0, const Position& p1, const Position& p2, const Position& px) {
    //Barycentric method

    auto const& x1 = p0.q1;
    auto const& y1 = p0.q2;
    auto const& x2 = p1.q1;
    auto const& y2 = p1.q2;
    auto const& x3 = p2.q1;
    auto const& y3 = p2.q2;
    auto const& x = px.q1;
    auto const& y = px.q2;

    double denominator = ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
    double a = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / denominator;
    double b = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / denominator;
    double c = 1.0 - a - b;
    return  0.0 <= a && a <= 1.0 &&
            0.0 <= b && b <= 1.0 &&
            0.0 <= c && c <= 1.0;
}

void AbstractTradeStrategy::trade(Simulation::EdgeworthSituation& situation,
                                      Simulation::ActorRef& actor1, Simulation::ActorRef& actor2)
{
    actor1.q1 = situation.result.q1;
    actor1.q2 = situation.result.q2;
    actor2.q1 = situation.q1Sum - actor1.q1;
    actor2.q2 = situation.q2Sum - actor1.q2;
}
