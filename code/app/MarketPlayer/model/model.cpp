#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <ctime>
#include <random>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cmath>

#include <iostream>

#include <unordered_map>
#include <vector>
#include <tuple>
#include <list>

#include <QVector>

#include "qcustomplot.h"
#include "model.h"

URNG globalUrng;

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

EdgeworthSituation::EdgeworthSituation(const Simulation& simulation, const size_t actor1Idx, const size_t actor2Idx,
        AbstractOfferStrategy& offerStrategy, AbstractAcceptanceStrategy& acceptanceStrategy, URNG& rng)
    : actor1(simulation, actor1Idx)
    , actor2(simulation, actor2Idx)
    , curve1(simulation.utility, actor1.q1, actor1.q2)
    , curve2(simulation.utility, actor2.q1, actor2.q2)
    , q1Sum(actor1.q1 + actor2.q1)
    , q2Sum(actor1.q2 + actor2.q2)
    , result(offerStrategy.propose(*this, rng))
    , successful(isItSuccessful(acceptanceStrategy.consider(*this), simulation.getMinSumTrade()))
{
}

bool EdgeworthSituation::isItSuccessful(bool consideration, Amount_t minimum) const
{
    Position const p0{actor1.q1, actor1.q2};
    Position const traded = result - p0;
    Amount_t const sumTraded = std::abs(traded.q1) + std::abs(traded.q2);
    return consideration && (sumTraded >= minimum);
}

CurveFunction EdgeworthSituation::getCurve1Function() const {
    return [this](double q1){ return curve1.getQ2(q1); };
}

CurveFunction EdgeworthSituation::getCurve2Function() const {
    return [this](double q1){ return q2Sum - curve2.getQ2(q1Sum - q1); };
}

//implicit hack: linear contract curve
CurveFunction EdgeworthSituation::getParetoSetFunction() const {
    return [this](double q1){ return q1 * q2Sum/q1Sum; };
}

//implicit hack of linear contract-curve
Amount_t EdgeworthSituation::calculateParetoIntersectionQ1(const IndifferenceCurve& curve) const {
    double const alfa1 = curve.utility.alfa1;
    double const alfa2 = curve.utility.alfa2;
    return pow(q1Sum/q2Sum * curve.fixP.q2 * pow(curve.fixP.q1, alfa1/alfa2),
               alfa2/(alfa1+alfa2));
}

Position EdgeworthSituation::calculateCurve1ParetoIntersection() const {
    Amount_t const q1 = calculateParetoIntersectionQ1(curve1);
    Amount_t const q2 = getCurve1Function()(q1);
    return Position{q1, q2};
}

//implicit hack of linear contract curve
Position EdgeworthSituation::calculateCurve2ParetoIntersection() const {
    Amount_t const q1 = q1Sum - calculateParetoIntersectionQ1(curve2);
    Amount_t const q2 = getCurve2Function()(q1);
    return Position{q1, q2};
}

Position EdgeworthSituation::calculateActor2Result() const
{
    return {q1Sum - result.q1, q2Sum - result.q2};
}

Amount_t EdgeworthSituation::calculateOriginalUtility(const Simulation::ActorConstRef &actor) const
{
    return curve1.utility.compute(actor.q1, actor.q2);
}

Amount_t EdgeworthSituation::calculateNewUtilityActor1() const
{
    return curve1.utility.compute(result.q1, result.q2);
}

Amount_t EdgeworthSituation::calculateNewUtilityActor2() const
{
    Position const actor2NewPos = calculateActor2Result();
    return curve2.utility.compute(actor2NewPos.q1, actor2NewPos.q2);
}

void Simulation::Progress::setup(size_t numActors, URNG& rng) {
    permutation.resize(numActors);
    std::generate(permutation.begin(), permutation.end(), IndexNumber());
    shufflePermutation(rng);
    actIdx = 0;
    restarted = false;
}

tuple<size_t, size_t> Simulation::Progress::getCurrentPair() const
{
    return make_tuple(permutation[actIdx], permutation[actIdx+1]);
}

bool Simulation::Progress::advance(URNG& rng)
{
    restarted = false;
    actIdx += 2;
    bool const finished = isFinished();
    if (finished) {
        shufflePermutation(rng);
        actIdx = 0;
        restarted = true;
    }
    return finished;
}

void Simulation::Progress::shufflePermutation(URNG &rng) {
    std::shuffle(permutation.begin(), permutation.end(), rng);
}

bool Simulation::Progress::isFinished() const
{
    return actIdx == permutation.size();
}

Simulation &Simulation::operator=(const Simulation &o)
{
    seed = o.seed;
    innerUrng = o.innerUrng;
    history = o.history;
    progress = o.progress;
    utility = o.utility;
    resources = o.resources;
    numActors = o.numActors;
    amounts = o.amounts;
    roundInfo = o.roundInfo;
    q2Price = o.q2Price;
    minTradeFactor = o.minTradeFactor;
    maxRoundWithoutTrade = o.maxRoundWithoutTrade;
    minSumTrade = o.minSumTrade;

    if (o.offerStrategy.get()) {
        offerStrategy.reset(o.offerStrategy->clone());
    } else {
        offerStrategy.reset();
    }
    if (o.acceptanceStrategy) {
        acceptanceStrategy.reset(o.acceptanceStrategy->clone());
    } else {
        acceptanceStrategy.reset();
    }

    //Intentionally reset to zero as this is just a view.
    //However, warning: if Simulation gets copied while in the middle of a step-by-step Edgeworth,
    //then (sigh) we will lose the random and the simulation will continue going a different course.
    previewedSituation.reset();

    return *this;
}

void Simulation::setupResources(vector<Amount_t>& targetResources, const Amount_t sumAmount, const size_t numActors) {
    targetResources.resize(numActors);
    std::generate_n(targetResources.begin(), numActors, [this](){
        std::uniform_real_distribution<Amount_t> uniformDistribution(0.0, 1.0);
        return uniformDistribution(innerUrng);
    });
    Amount_t const sumRandom = std::accumulate(targetResources.begin(), targetResources.end(), 0.0);
    double const ratio = sumAmount/sumRandom;
    for (auto& element : targetResources) {
        element *= ratio;
    }
}

bool Simulation::setup(
        URNG::result_type seed,
        size_t numActors,
        unsigned amountQ1, unsigned amountQ2,
        double alfa1, double alfa2,
        double minTradeFactor, size_t maxRoundWithoutTrade) {
    if (numActors%2 != 0) return false;
    this->seed = seed;
    innerUrng.seed(seed);
    history.reset();
    amounts.resize(0);
    amounts.push_back(amountQ1);
    amounts.push_back(amountQ2);
    resources.resize(amounts.size());
    this->numActors = numActors;
    utility.alfa1 = alfa1;
    utility.alfa2 = alfa2;
    this->minTradeFactor = minTradeFactor;
    this->maxRoundWithoutTrade = maxRoundWithoutTrade;
    this->minSumTrade = calculateMinSumTrade(amounts[0], amounts[1], numActors, minTradeFactor);
    progress.setup(numActors, innerUrng);
    for (vector<Amount_t>::size_type idx = 0; idx < amounts.size(); ++idx) {
        setupResources(resources[idx], amounts[idx], numActors);
    }
    q2Price = amounts[0] / amounts[1];
    roundInfo.reset();
    saveHistory();
    return true;
}

EdgeworthSituation Simulation::getNextSituation() const
{
    size_t actor1Idx, actor2Idx;
    std::tie(actor1Idx, actor2Idx) = progress.getCurrentPair();
    return EdgeworthSituation(*this, actor1Idx, actor2Idx, *offerStrategy, *acceptanceStrategy, innerUrng);
}

Amount_t Simulation::calculateMinSumTrade(Amount_t sumQ1, Amount_t sumQ2, size_t numActors, Amount_t minTradeFactor)
{
    return (sumQ1 + sumQ2) / static_cast<Amount_t>(numActors) * minTradeFactor;
}

EdgeworthSituation const& Simulation::provideNextSituation()
{
    if (!previewedSituation.get()) {
        size_t actor1Idx, actor2Idx;
        std::tie(actor1Idx, actor2Idx) = progress.getCurrentPair();
        previewedSituation.reset(new EdgeworthSituation(*this, actor1Idx, actor2Idx,
                                                        *offerStrategy, *acceptanceStrategy, innerUrng));
    }
    return *previewedSituation;
}

Position Simulation::trade(EdgeworthSituation const& situation, ActorRef& actor1, ActorRef& actor2)
{
    Position traded { actor1.q1, actor1.q2 };
    traded = traded - situation.result;
    actor1.q1 = situation.result.q1;
    actor1.q2 = situation.result.q2;
    Position actor2NewPos = situation.calculateActor2Result();
    actor2.q1 = actor2NewPos.q1;
    actor2.q2 = actor2NewPos.q2;
    return traded;
}

Amount_t Simulation::computeWealth(Position position) const
{
    return position.q1 + position.q2*q2Price;
}

void Simulation::saveHistory()
{
    history.q1Traded.push(roundInfo.q1Traded);
    history.q2Traded.push(roundInfo.q2Traded);
    history.numSuccessful.push(roundInfo.numSuccessful);

    auto& moment = history.newMoment();

    //todo generalize
    Amount_t const q1Resolution = amounts[0] / numActors / 8;
    moment.q1Distribution.setup(resources[0], q1Resolution);

    Amount_t const q2Resolution = amounts[1] / numActors / 8;
    moment.q2Distribution.setup(resources[1], q2Resolution);

    Amount_t const utilityResolution = utility.compute(amounts[0],amounts[1]) / numActors / 8;
    auto const& utilities = computeActors(
        [this](ActorConstRef const& actor) {
            return utility.compute(actor.q1, actor.q2);
        }
    );
    moment.utilityDistribution.setup(utilities, utilityResolution);
    auto sumUtilities = std::accumulate(utilities.begin(), utilities.end(), 0.0);
    history.sumUtilities.push(sumUtilities);

    Amount_t const wealthResolution = (amounts[0] + amounts[1]*q2Price) / numActors / 8;
    auto const& wealth = computeActors(
        [this](ActorConstRef const& actor) {
            return computeWealth({actor.q1, actor.q2});
        }
    );
    moment.wealthDistribution.setup(wealth, wealthResolution);

    history.wealthDeviation.push(moment.wealthDistribution.standardDeviation);
}

Amount_t Simulation::getMinSumTrade() const
{
    return minSumTrade;
}

bool Simulation::performNextTrade()
{
    size_t actor1Idx, actor2Idx;
    std::tie(actor1Idx, actor2Idx) = progress.getCurrentPair();
    EdgeworthSituation const& situation = previewedSituation.get() ?
                *previewedSituation : getNextSituation();
    if (situation.successful) {
        ActorRef actor1(*this, actor1Idx);
        ActorRef actor2(*this, actor2Idx);
        Position traded = trade(situation, actor1, actor2);
        roundInfo.recordTrade(traded);
    }
    previewedSituation.reset();
    bool const progressFinished = progress.advance(innerUrng);
    if (progressFinished) {
        saveHistory();
        roundInfo.reset();
    }
    return progressFinished;
}

void Simulation::performNextRound()
{
    while (!performNextTrade());
}

vector<Amount_t> Simulation::computeActors(std::function<Amount_t(ActorConstRef const&)> evaluatorFn) const
{
    vector<Amount_t> result;
    result.reserve(numActors);
    for (size_t actorIdx = 0; actorIdx < numActors; ++actorIdx) {
        ActorConstRef actor(*this, actorIdx);
        result.push_back(evaluatorFn(actor));
    }
    return result;
}

History::History(): time(0) {}

Moment& History::newMoment()
{
    ++time;
    moments.push_back(Moment());
    return moments.back();
}

void History::reset()
{
    time = 0;
    q1Traded.reset();
    q2Traded.reset();
    numSuccessful.reset();
    sumUtilities.reset();
    wealthDeviation.reset();
    moments.resize(0);
}

size_t History::size() const
{
    return time;
}

void Simulation::RoundInfo::reset()
{
    q1Traded = 0;
    q2Traded = 0;
    numSuccessful = 0;
}

void Simulation::RoundInfo::recordTrade(Position traded)
{
    q1Traded += std::abs(traded.q1);
    q2Traded += std::abs(traded.q2);
    numSuccessful += 1;
}
