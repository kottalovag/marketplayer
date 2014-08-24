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

#include "qcustomplot.h"

typedef std::mt19937 URNG;
extern URNG urng;

using std::tuple;

template<typename E>
//using vector = std::vector<E>;
using vector = QVector<E>;

typedef double Amount_t;

struct Simulation;

struct IndexNumber
{
    unsigned int actual;
    IndexNumber()
        :actual(0)
    {}
    unsigned int operator()() {return actual++;}
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
    Amount_t fixQ1, fixQ2;

    IndifferenceCurve(Utility utility, Amount_t q1, Amount_t q2);
    ResourceDataPair getFixPoint() const;
    Amount_t getQ2(Amount_t q1) const;
};

struct ResourceToleranceEquality
{
    bool operator()(Amount_t const& x, Amount_t const& y) const;
};

typedef std::unordered_map<Amount_t, bool, std::hash<Amount_t>, ResourceToleranceEquality> PinPointMap;

ResourceDataPair sampleFunction(std::function<double(double)> func, Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution);

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
        ActorRef actor1, actor2;
        IndifferenceCurve curve1, curve2;
        Amount_t const q1Sum, q2Sum;
        EdgeworthSituation(Simulation& simulation, size_t const actor1Idx, size_t const actor2Idx);
        std::function<Amount_t(Amount_t)> getCurve1Function() const;
        std::function<Amount_t(Amount_t)> getCurve2Function() const;
        std::function<Amount_t(Amount_t)> getParetoSetFunction() const;

        ResourceDataPair getFixPoint() const {
            return curve1.getFixPoint();
        }

        Amount_t calculateCurve1ParetoIntersection() const;
        Amount_t calculateCurve2ParetoIntersection() const;

    private:
        Amount_t calculateParetoIntersection(IndifferenceCurve const& curve) const;
    };

    Utility utility;
    vector<vector<Amount_t>> resources;
    vector<size_t> permutation;
    size_t numActors;
    vector<Amount_t> amounts;

    void setupPermutation();
    void shufflePermutation();
    bool checkResources(size_t resourceIdx);
    void printResources(size_t resourceIdx);
    static void setupResources(vector<Amount_t>& resources, Amount_t const sumAmount, size_t const numActors);
    bool setup(size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2);
    void nextRound();

    vector<Amount_t> computeUtilities() const;
};

struct Distribution {
    vector<Amount_t> const& subject;
    Amount_t const resolution;
    Amount_t const max;
    size_t const numBuckets;

    ResourceDataPair data;

    Distribution(vector<Amount_t> const& subject, Amount_t resolution);
};
