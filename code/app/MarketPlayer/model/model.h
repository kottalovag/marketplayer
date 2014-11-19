#pragma once

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
#include <memory>

#include "modelutils.h"
#include "strategy.h"

using std::tuple;
using std::unique_ptr;

struct Simulation;

struct Utility
{
    double alfa1, alfa2;
    Amount_t compute(Amount_t q1, Amount_t q2) const;
};

struct IndifferenceCurve
{
    Utility utility;
    Position fixP;

    IndifferenceCurve(Utility utility, Amount_t q1, Amount_t q2);
    Amount_t getQ2(Amount_t q1) const;
};

typedef std::unordered_map<Amount_t, bool, std::hash<Amount_t>, ResourceToleranceEquality> PinPointMap;

struct Moment
{
    HeavyDistribution q1Distribution, q2Distribution, utilityDistribution, wealthDistribution;
};

struct History
{
    History();
    size_t time;
    vector<Moment> moments;
    DataTimePair q1Traded, q2Traded, numSuccessful, sumUtilities, wealthDeviation;
    Moment& newMoment();
    void reset();
    size_t size() const;
};

struct AbstractOfferStrategy;
struct AbstractAcceptanceStrategy;

struct Simulation
{
    struct ActorConstRef {
        Amount_t const& q1;
        Amount_t const& q2;
        ActorConstRef(Simulation const& simulation, size_t const idx)
            : q1(simulation.resources[0][idx])
            , q2(simulation.resources[1][idx])
        {}
    };

    struct ActorRef {
        Amount_t& q1;
        Amount_t& q2;
        ActorRef(Simulation& simulation, size_t const idx)
            : q1(simulation.resources[0][idx])
            , q2(simulation.resources[1][idx])
        {}
    };

    struct Progress
    {
        void setup(size_t numActor, URNG &rng);
        tuple<size_t, size_t> getCurrentPair() const;
        bool advance(URNG &rng);
        size_t getDone() const { return actIdx / 2; }
        size_t getNum() const { return permutation.size() / 2; }
        bool wasRestarted() const { return restarted; }

    private:
        bool isFinished() const;
        bool restarted;

    private:
        void shufflePermutation(URNG& rng);

        vector<size_t> permutation;
        vector<size_t>::size_type actIdx;
    };

    struct RoundInfo
    {
        void reset();
        void recordTrade(Position traded);
        Amount_t q1Traded, q2Traded, numSuccessful;
    };

    URNG::result_type seed;
    mutable URNG innerUrng;
    History history;
    Progress progress;
    Utility utility;
    vector<vector<Amount_t>> resources;
    size_t numActors;
    vector<Amount_t> amounts;
    RoundInfo roundInfo;
    double q2Price;
    double minTradeFactor;
    size_t maxRoundWithoutTrade;

    unique_ptr<AbstractOfferStrategy> offerStrategy;
    unique_ptr<AbstractAcceptanceStrategy> acceptanceStrategy;

    Simulation(){}
    Simulation(Simulation const& o) { *this = o; }
    Simulation& operator=(Simulation const& o);

    Amount_t getSumQ1() const { return amounts[0]; }
    Amount_t getSumQ2() const { return amounts[1]; }
    size_t getNumMaxTrade() const { return numActors/2; }

    void setupResources(vector<Amount_t>& targetResources, Amount_t const sumAmount, size_t const numActors);
    bool setup(
            URNG::result_type seed,
            size_t numActors,
            unsigned amountQ1, unsigned amountQ2,
            double alfa1, double alfa2,
            double minTradeFactor, size_t maxRoundWithoutTrade);
    bool performNextTrade();
    void performNextRound();
    const EdgeworthSituation &provideNextSituation();
    Position trade(const EdgeworthSituation &situation, ActorRef& actor1, ActorRef& actor2);

    Amount_t computeWealth(Position position) const;
    vector<Amount_t> computeActors(std::function<Amount_t(ActorConstRef const&)> evaluatorFn) const;
    void saveHistory();
    Amount_t getMinSumTrade() const;

    static Amount_t calculateMinSumTrade(Amount_t sumQ1, Amount_t sumQ2, size_t numActors, Amount_t minTradeFactor);
private:
    unique_ptr<EdgeworthSituation> previewedSituation;
    EdgeworthSituation getNextSituation() const;
    Amount_t minSumTrade;
};

struct EdgeworthSituation {
    Simulation::ActorConstRef actor1, actor2;
    IndifferenceCurve const curve1, curve2;
    Amount_t const q1Sum, q2Sum;
    Position const result;
    bool const successful;

    EdgeworthSituation(Simulation const& simulation, size_t const actor1Idx, size_t const actor2Idx,
                       AbstractOfferStrategy& offerStrategy, AbstractAcceptanceStrategy &acceptanceStrategy, URNG &rng);

    bool isItSuccessful(bool consideration, Amount_t minimum) const;
    CurveFunction getCurve1Function() const;
    CurveFunction getCurve2Function() const;
    CurveFunction getParetoSetFunction() const;

    Position getFixPoint() const {
        return curve1.fixP;
    }

    Position calculateCurve1ParetoIntersection() const;
    Position calculateCurve2ParetoIntersection() const;
    Position calculateActor2Result() const;
    Amount_t calculateOriginalUtility(Simulation::ActorConstRef const& actor) const;
    Amount_t calculateNewUtilityActor1() const;
    Amount_t calculateNewUtilityActor2() const;

private:
    Amount_t calculateParetoIntersectionQ1(IndifferenceCurve const& curve) const;
};
