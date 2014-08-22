#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <ctime>
#include <random>
#include <algorithm>
#include <numeric>

#include <iostream>

#include <unordered_map>
#include <vector>
#include <tuple>
#include <list>

#include "qcustomplot.h"

using namespace std;

typedef std::mt19937 URNG;
URNG urng;

struct IndexNumber
{
    unsigned int actual;
    IndexNumber() {
        actual = 0;
    }
    unsigned int operator()() {return actual++;}
};

typedef double Amount_t;

struct Utility
{
    double alfa1, alfa2;
    Amount_t compute(Amount_t q1, Amount_t q2) const {
        return pow(q1, alfa1)*pow(q2, alfa2);
    }
    Amount_t computeQ1(Amount_t q2, Amount_t product) const {
        return computeOther(q2, alfa2, alfa1, product);
    }
    Amount_t computeQ2(Amount_t q1, Amount_t product) const {
        return computeOther(q1, alfa1, alfa2, product);
    }
private:
    static Amount_t computeOther(Amount_t qGiven, double alfaGiven, double alfaSubject, Amount_t product) {
        auto qSubject_alfaSubject = product/pow(qGiven,alfaGiven);
        return pow(qSubject_alfaSubject, 1.0/alfaSubject);
    }
};

struct IndifferenceCurve
{
    Utility utility;
    Amount_t q1, q2;
    typedef tuple<vector<Amount_t>, vector<Amount_t>> DataPair;

    DataPair computeCurve(Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution) const{
        auto const product = utility.compute(q1,q2);
        vector<Amount_t> q1Points, q2Points;
        for (auto loopQ1 = rangeStart; loopQ1 <= rangeFinish; loopQ1 += resolution) {
            q1Points.push_back(loopQ1);
            q2Points.push_back(utility.computeQ2(loopQ1, product));
        }
        return make_tuple(q1Points, q2Points);
    }

    static void flipCurve(DataPair& curve, Amount_t q1Sum, Amount_t q2Sum) {
        auto& q1Points = std::get<0>(curve);
        auto& q2Points = std::get<1>(curve);
        Q_ASSERT(q1Points.size() == q2Points.size());
        for (size_t i = 0; i < q1Points.size(); ++i) {
            q1Points[i] = q1Sum - q1Points[i];
            q2Points[i] = q2Sum - q2Points[i];
        }
    }
};

struct Simulation
{
    Utility mUtility;
    vector<vector<Amount_t>> mResources;
    vector<size_t> mPermutation;
    size_t mNumActors;
    vector<Amount_t> mAmounts;

    void setupPermutation() {
        mPermutation.resize(mNumActors);
        generate(mPermutation.begin(), mPermutation.end(), IndexNumber());
    }

    void shufflePermutation() {
        shuffle(mPermutation.begin(), mPermutation.end(), urng);
    }

    bool checkResources(size_t resourceIdx) {
        auto const& res = mResources[resourceIdx];
        Amount_t const sum = std::accumulate(res.begin(), res.end(), 0.0);
        return sum == mAmounts[resourceIdx];
        //return fabs(sum - mAmounts[resourceIdx]) < numeric_limits<Amount_t>::epsilon();
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

    static void setupResourcesInt(vector<Amount_t>& resources, Amount_t const sumAmount, size_t numActors) {
        Q_ASSERT(numActors <= sumAmount);
        resources.reserve(numActors);
        resources.resize(0);
        unordered_map<Amount_t, bool> isPinPointUsed;
        vector<Amount_t>& pinPoints = resources; //alias for readability
        for (Amount_t upperBound = sumAmount - numActors; upperBound < sumAmount; ++upperBound) {
            std::uniform_int_distribution<Amount_t> uniformDistribution(0, upperBound);
            Amount_t pinPoint = uniformDistribution(urng);
            if (isPinPointUsed[pinPoint]) {
                pinPoint = upperBound;
            }
            isPinPointUsed[pinPoint] = true;
            pinPoints.push_back(pinPoint);
        }
        std::sort(pinPoints.begin(), pinPoints.end());
        Q_ASSERT(numActors == pinPoints.size());

        //resources and pinPoints is the same container
        //the alias was used for sake of clarity
        //we have to be careful about the order
        Amount_t const amountAtBorder = pinPoints[0] + (sumAmount - pinPoints.back());
        for (size_t actorIdx = numActors-1; actorIdx > 0; --actorIdx) {
            resources[actorIdx] = pinPoints[actorIdx] - pinPoints[actorIdx-1];
        }
        resources[0] = amountAtBorder;
    }

    static void setupResourcesReal(vector<Amount_t>& resources, Amount_t const sumAmount, size_t numActors) {
        resources.reserve(numActors);
        resources.resize(0);
        unordered_map<Amount_t, bool> isPinPointUsed;
        vector<Amount_t>& pinPoints = resources; //alias for readability
        for (Amount_t upperBound = sumAmount - numActors; upperBound < sumAmount; ++upperBound) {
            std::uniform_int_distribution<Amount_t> uniformDistribution(0, upperBound);
            Amount_t pinPoint = uniformDistribution(urng);
            if (isPinPointUsed[pinPoint]) {
                pinPoint = upperBound;
            }
            isPinPointUsed[pinPoint] = true;
            pinPoints.push_back(pinPoint);
        }
        std::sort(pinPoints.begin(), pinPoints.end());
        Q_ASSERT(numActors == pinPoints.size());

        //resources and pinPoints is the same container
        //the alias was used for sake of clarity
        //we have to be careful about the order
        Amount_t const amountAtBorder = pinPoints[0] + (sumAmount - pinPoints.back());
        for (size_t actorIdx = numActors-1; actorIdx > 0; --actorIdx) {
            resources[actorIdx] = pinPoints[actorIdx] - pinPoints[actorIdx-1];
        }
        resources[0] = amountAtBorder;
    }

    bool setup(size_t numActors, unsigned amountQ1, unsigned amountQ2, double alfa1, double alfa2)
    {
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
            setupResourcesInt(mResources[idx], mAmounts[idx], numActors);
            Q_ASSERT(checkResources(idx));
        }
        return true;
    }

    tuple<Amount_t, Amount_t> proposeTrade(size_t proposerActorIdx, size_t targetActorIdx) {

        Amount_t tradedQ1, tradedQ2;
    }

    void nextRound()
    {
        shufflePermutation();
        for (size_t idx = 0; idx < mNumActors; idx += 2) {
            auto const oneActorIdx = mPermutation[idx];
            auto const otherActorIdx = mPermutation[idx+1];
            //trade(oneActorIdx, otherActorIdx);
        }
    }

    void plotEdgeWorth(QCustomPlot* plot, size_t actorIdx1, size_t actorIdx2)
    {


    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    urng.seed(std::time(0));
    ui->setupUi(this);
    Simulation simulation;
    simulation.setup(10, 1000, 800, 0.5, 0.5);
    simulation.printResources(0);
    simulation.printResources(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
