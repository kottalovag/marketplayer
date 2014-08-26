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
};

typedef DataPair<Amount_t> ResourceDataPair;

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

struct History
{
    History(): time(0) {}
    size_t time;
    void roundFinished() { ++time; }
};

struct AbstractTradeStrategy;

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

        EdgeworthSituation(Simulation const& simulation, size_t const actor1Idx, size_t const actor2Idx,
                           AbstractTradeStrategy& tradeStrategy);
        std::function<Amount_t(Amount_t)> getCurve1Function() const;
        std::function<Amount_t(Amount_t)> getCurve2Function() const;
        std::function<Amount_t(Amount_t)> getParetoSetFunction() const;

        Position getFixPoint() const {
            return curve1.fixP;
        }

        Position calculateCurve1ParetoIntersection() const;
        Position calculateCurve2ParetoIntersection() const;

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

    History history;
    Progress progress;
    Utility utility;
    vector<vector<Amount_t>> resources;
    size_t numActors;
    vector<Amount_t> amounts;

    unique_ptr<AbstractTradeStrategy> tradeStrategy;

    static void setupResources(vector<Amount_t>& resources, Amount_t const sumAmount, size_t const numActors);
    bool setup(size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2);
    bool performNextTrade();
    void performNextRound();
    EdgeworthSituation getNextSituation() const;

    vector<Amount_t> computeUtilities() const;
};

struct AbstractTradeStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation& situation) const = 0;
    void trade(Simulation::EdgeworthSituation& situation,
                   Simulation::ActorRef& actor1, Simulation::ActorRef& actor2);
    virtual ~AbstractTradeStrategy(){}
};

struct OppositeParetoTradeStrategy: AbstractTradeStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation &situation) const override;
    virtual ~OppositeParetoTradeStrategy() {}
};

struct RandomParetoTradeStrategy: AbstractTradeStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation &situation) const override;
    virtual ~RandomParetoTradeStrategy() {}
};

bool isPointInTriangle(Position const& p0, Position const& p1, Position const& p2, Position const& px);

struct RandomTriangleTradeStrategy: AbstractTradeStrategy
{
    virtual Position propose(Simulation::EdgeworthSituation &situation) const override;
    virtual ~RandomTriangleTradeStrategy() {}
};

struct Distribution {
    vector<Amount_t> const& subject;
    Amount_t const resolution;
    Amount_t const max;
    size_t const numBuckets;

    ResourceDataPair data;

    Distribution(vector<Amount_t> const& subject, Amount_t resolution);
};
