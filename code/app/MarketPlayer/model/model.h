#pragma once

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
#include <memory>

#include "qcustomplot.h"

typedef std::mt19937 URNG;
extern URNG urng;

using std::tuple;
using std::unique_ptr;

template<typename E>
//using vector = std::vector<E>;
using vector = QVector<E>;

typedef double Amount_t;

struct Simulation;

struct Position;
extern std::function<void(Position const&)> debugShowPoint;

struct IndexNumber
{
    unsigned int actual;
    IndexNumber()
        :actual(0)
    {}
    unsigned int operator()() {return actual++;}
};

struct Position
{
    Amount_t q1, q2;
    Position operator-(const Position& other) const {
        return Position{q1 - other.q1, q2 - other.q2};
    }
    Position operator+(const Position& other) const {
        return Position{q1 + other.q1, q2 + other.q2};
    }
    Position operator*(const double& factor) const {
        return Position{q1*factor, q2*factor};
    }
};

template<typename T>
struct DataPair
{
    typedef typename vector<T>::size_type size_type;
    vector<T> x,y;
    size_type size() const { return x.size(); }

    void push(T const& x, T const& y) {
        this->x.push_back(x);
        this->y.push_back(y);
    }

    void resize(size_type size) {
        x.resize(size);
        y.resize(size);
    }
    void reset() {
        resize(0);
    }
};

typedef DataPair<Amount_t> ResourceDataPair;

struct DataTimePair
{
    DataPair<Amount_t> data;
    Amount_t max;
    Amount_t getLastX() const { return data.x.size() - 1; }
    void push(Amount_t newData) {
        data.push(getLastX() + 1, newData);
        if (newData > max) {
            max = newData;
        }
    }
    void reset() {
        max = 0.0;
        data.reset();
    }
    Amount_t const& operator[](size_t idx) const {
        return data.y[idx];
    }
};

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

struct ResourceToleranceEquality
{
    bool operator()(Amount_t const& x, Amount_t const& y) const;
};

typedef std::unordered_map<Amount_t, bool, std::hash<Amount_t>, ResourceToleranceEquality> PinPointMap;

ResourceDataPair sampleFunction(std::function<double(double)> func, Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution);

struct Distribution {
    vector<Amount_t> const& subject;
    Amount_t const resolution;
    Amount_t const maxSubject;
    size_t const numBuckets;

    ResourceDataPair data;

    Distribution(vector<Amount_t> const& subject, Amount_t resolution);
};

struct HeavyDistribution
{
    Amount_t resolution;
    Amount_t maxSubject;
    Amount_t maxNum;
    size_t numBuckets;
    ResourceDataPair data;

    void setup(vector<Amount_t> const& subject, Amount_t resolution);
};

struct Moment
{
    HeavyDistribution q1Distribution, q2Distribution, utilityDistribution, wealthDistribution;
};

struct History
{
    History();
    size_t time;
    vector<Moment> moments;
    DataTimePair q1Traded, q2Traded, numSuccessful, sumUtilities;
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
                           AbstractOfferStrategy& offerStrategy, AbstractAcceptanceStrategy &acceptanceStrategy);
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
        void setup(size_t numActor);
        tuple<size_t, size_t> getCurrentPair() const;
        bool advance();
        size_t getDone() const { return actIdx / 2; }
        size_t getNum() const { return permutation.size() / 2; }
        bool wasRestarted() const { return restarted; }

    private:
        bool isFinished() const;
        bool restarted;

    private:
        void shufflePermutation();

        vector<size_t> permutation;
        size_t actIdx;
    };

    struct RoundInfo
    {
        void reset();
        void recordTrade(Position traded);
        Amount_t q1Traded, q2Traded, numSuccessful;
    };

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

    static void setupResources(vector<Amount_t>& resources, Amount_t const sumAmount, size_t const numActors);
    bool setup(size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2);
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
    virtual Position propose(Simulation::EdgeworthSituation const& situation) const = 0;
    virtual ~AbstractOfferStrategy(){}
};

struct OppositeParetoOfferStrategy: AbstractOfferStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation const& situation) const override;
    virtual ~OppositeParetoOfferStrategy() {}
};

struct RandomParetoOfferStrategy: AbstractOfferStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation const& situation) const override;
    virtual ~RandomParetoOfferStrategy() {}
};

bool isPointInTriangle(Position const& p0, Position const& p1, Position const& p2, Position const& px);

struct RandomTriangleOfferStrategy: AbstractOfferStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation const& situation) const override;
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
