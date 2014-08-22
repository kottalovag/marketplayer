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

struct Utility
{


};

struct Simulation
{
    typedef int32_t Amount_t;
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

    static void setupResources(vector<Amount_t>& resources, Amount_t const sumAmount, size_t numActors) {
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

    bool setup(size_t numActors, unsigned amountQ1, unsigned amountQ2)
    {
        if (numActors%2 != 0) return false;
        mAmounts.resize(0);
        mAmounts.push_back(amountQ1);
        mAmounts.push_back(amountQ2);
        mResources.resize(mAmounts.size());
        mNumActors = numActors;
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
    simulation.setup(10, 1000, 800);
    simulation.printResources(0);
    simulation.printResources(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
