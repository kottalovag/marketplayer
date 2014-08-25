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
    PinPointMap isPinPointUsed;
    vector<Amount_t> pinPoints;
    pinPoints.reserve(numActors);

    std::uniform_real_distribution<Amount_t> uniformDistribution(0.0, sumAmount);
    while (pinPoints.size() < numActors) {
        Amount_t pinPoint = uniformDistribution(urng);
        if (!isPinPointUsed[pinPoint]) {
            isPinPointUsed[pinPoint] = true;
            pinPoints.push_back(pinPoint);
        }
    }
    std::sort(pinPoints.begin(), pinPoints.end());

    resources.resize(numActors);
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
    this->numActors = numActors;
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

Position OppositeParetoTradeStrategy::propose(Simulation::EdgeworthSituation& situation)
{
    Position const p2 = situation.calculateCurve2ParetoIntersection();
    debugShowPoint(p2);
    return p2;
}

Position RandomParetoTradeStrategy::propose(Simulation::EdgeworthSituation& situation)
{
    Position const p2 = situation.calculateCurve2ParetoIntersection();
    Position const p1 = situation.calculateCurve1ParetoIntersection();
    double const factor = std::uniform_real_distribution<double>(0.0, 1.0)(urng);
    auto const result = p1 + (p2 - p1) * factor;
    debugShowPoint(result);
    return result;
}

Position RandomTriangleTradeStrategy::propose(Simulation::EdgeworthSituation& situation)
{
    Position const p0 = situation.getFixPoint();
    Position const p1 = situation.calculateCurve1ParetoIntersection();
    Position const p2 = situation.calculateCurve2ParetoIntersection();
    auto const v01 = (p1 - p0);
    auto const v02 = (p2 - p0);

    std::uniform_real_distribution<double> udist(0.0, 1.0);

    //we pick a point as if in a paralelogram for sake of uniformity
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
    debugShowPoint(px);
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

Position AbstractTradeStrategy::trade(Simulation::EdgeworthSituation& situation)
{
    auto proposal = propose(situation);
    situation.actor1.q1 = proposal.q1;
    situation.actor1.q2 = proposal.q2;
    return proposal;
}
