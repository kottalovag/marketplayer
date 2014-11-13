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

    struct EdgeworthSituation {
        ActorConstRef actor1, actor2;
        IndifferenceCurve const curve1, curve2;
        Amount_t const q1Sum, q2Sum;
        Position const result;
        bool const successful;

        EdgeworthSituation(Simulation const& simulation, size_t const actor1Idx, size_t const actor2Idx,
                           AbstractOfferStrategy& offerStrategy, AbstractAcceptanceStrategy &acceptanceStrategy, URNG &rng);
        std::function<Amount_t(Amount_t)> getCurve1Function() const;
        std::function<Amount_t(Amount_t)> getCurve2Function() const;
        std::function<Amount_t(Amount_t)> getParetoSetFunction() const;

        Position getFixPoint() const {
            return curve1.fixP;
        }

        Position calculateCurve1ParetoIntersection() const;
        Position calculateCurve2ParetoIntersection() const;
        Position calculateActor2Result() const;
        Amount_t calculateOriginalUtility(ActorConstRef const& actor) const;
        Amount_t calculateNewUtilityActor1() const;
        Amount_t calculateNewUtilityActor2() const;

    private:
        Amount_t calculateParetoIntersectionQ1(IndifferenceCurve const& curve) const;
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
        size_t actIdx;
    };

    struct RoundInfo
    {
        void reset();
        void recordTrade(Position traded);
        Amount_t q1Traded, q2Traded, numSuccessful;
    };

    mutable URNG innerUrng;
    History history;
    Progress progress;
    Utility utility;
    vector<vector<Amount_t>> resources;
    size_t numActors;
    vector<Amount_t> amounts;
    RoundInfo roundInfo;
    double q2Price;

    unique_ptr<AbstractOfferStrategy> offerStrategy;
    unique_ptr<AbstractAcceptanceStrategy> acceptanceStrategy;

    void setupResources(vector<Amount_t>& targetResources, Amount_t const sumAmount, size_t const numActors);
    bool setup(int seed, size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2);
    bool performNextTrade();
    void performNextRound();
    const EdgeworthSituation &provideNextSituation();
    Position trade(const EdgeworthSituation &situation, ActorRef& actor1, ActorRef& actor2);

    Amount_t computeWealth(Position position) const;
    vector<Amount_t> computeActors(std::function<Amount_t(ActorConstRef const&)> evaluatorFn) const;
    void saveHistory();

private:
    unique_ptr<EdgeworthSituation> previewedSituation;
    EdgeworthSituation getNextSituation() const;
};

struct AbstractOfferStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation const& situation, URNG& rng) const = 0;
    virtual ~AbstractOfferStrategy(){}
};

struct OppositeParetoOfferStrategy: AbstractOfferStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation const& situation, URNG& rng) const override;
    virtual ~OppositeParetoOfferStrategy() {}
};

struct RandomParetoOfferStrategy: AbstractOfferStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation const& situation, URNG &rng) const override;
    virtual ~RandomParetoOfferStrategy() {}
};

struct RandomTriangleOfferStrategy: AbstractOfferStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation const& situation, URNG& rng) const override;
    virtual ~RandomTriangleOfferStrategy() {}
};

struct AbstractAcceptanceStrategy
{
    virtual bool consider(Simulation::EdgeworthSituation const& situation) const = 0;
    virtual ~AbstractAcceptanceStrategy(){}
    bool considerGeneral(const Simulation::EdgeworthSituation &situation,
                         std::function<Amount_t(Amount_t const&, Amount_t const&)> evaluate) const;
};

struct AlwaysAcceptanceStrategy: AbstractAcceptanceStrategy
{
    virtual bool consider(Simulation::EdgeworthSituation const& situation) const override;
    virtual ~AlwaysAcceptanceStrategy(){}
};

struct HigherGainAcceptanceStrategy: AbstractAcceptanceStrategy
{
    virtual bool consider(Simulation::EdgeworthSituation const& situation) const override;
    virtual ~HigherGainAcceptanceStrategy(){}
};

struct HigherProportionAcceptanceStrategy: AbstractAcceptanceStrategy
{
    virtual bool consider(Simulation::EdgeworthSituation const& situation) const override;
    virtual ~HigherProportionAcceptanceStrategy(){}
};
