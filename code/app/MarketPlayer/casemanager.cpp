#include "casemanager.h"
#include <algorithm>

CaseManager::CaseManager(
        DataTimeRatioPlot* plotQ1Traded,
        DataTimeRatioPlot* plotQ2Traded,
        DataTimePlot* plotSumUtility,
        DataTimeRatioPlot* plotNumSuccessfulTrades,
        DataTimePlot* plotWealthDeviation,
        DistributionPlot* plotQ1Distribution,
        DistributionPlot* plotQ2Distribution,
        DistributionPlot* plotUtilityDistribution,
        DistributionPlot* plotWealthDistribution
        )
    :   plotQ1Traded(plotQ1Traded)
    ,   plotQ2Traded(plotQ2Traded)
    ,   plotSumUtility(plotSumUtility)
    ,   plotNumSuccessfulTrades(plotNumSuccessfulTrades)
    ,   plotWealthDeviation(plotWealthDeviation)
    ,   plotQ1Distribution(plotQ1Distribution)
    ,   plotQ2Distribution(plotQ2Distribution)
    ,   plotUtilityDistribution(plotUtilityDistribution)
    ,   plotWealthDistribution(plotWealthDistribution)
    ,   dataTimePlots({plotQ1Traded, plotQ2Traded, plotSumUtility, plotNumSuccessfulTrades, plotWealthDeviation})
    ,   distributionPlots({plotQ1Distribution, plotQ2Distribution, plotUtilityDistribution, plotWealthDistribution})
    ,   currentDataIdx(0)
{
    plots.insert(plots.end(), dataTimePlots.begin(), dataTimePlots.end());
    plots.insert(plots.end(), distributionPlots.begin(), distributionPlots.end());
}

bool CaseManager::contains(QString caseName) const
{
    return simulationCases.find(caseName) != simulationCases.end();
}

const AbstractSimulationCase &CaseManager::getSimulationCase(QString caseName) const
{
    return *simulationCases.find(caseName)->second;
}

size_t CaseManager::calculateLastVisibleDataIdx() const
{
    size_t resultIdx = 0;
    for (auto const& simulationCasePair : simulationCases) {
        auto const& simulationCase = simulationCasePair.second;
        if (simulationCase->isShown) {
            auto const idx = simulationCase->getSimulation().history.size()-1;
            if (idx > resultIdx) {
                resultIdx = idx;
            }
        }
    }
    return resultIdx;
}

size_t CaseManager::getValidCurrentDataIdx() const
{
    return currentDataIdx;
}

void CaseManager::addHeavyCase(QString caseName, const Simulation &simulation, QColor color, bool visible)
{
    addCase<HeavySimulationCase>(simulation, caseName, color, visible);
}

//The caller has to ensure the passed simulation outlives the manager
void CaseManager::addExternalCase(QString caseName, const Simulation &simulation, QColor color, bool visible)
{
    addCase<ExternalSimulationCase>(simulation, caseName, color, visible);
}

void CaseManager::removeCase(QString caseName)
{
    auto foundElement = simulationCases.find(caseName);
    if (foundElement != simulationCases.end()) {
        auto& simulationCase = foundElement->second;
        dropBundles(*simulationCase);
    }
    simulationCases.erase(foundElement);
    updatePlots();
}

void CaseManager::setVisibility(QString caseName, bool visible)
{
    auto& simulationCase = simulationCases[caseName];
    if (simulationCase->isShown != visible) {
        simulationCase->isShown = visible;
        if (visible) {
            setupBundles(*simulationCase);
        } else {
            dropBundles(*simulationCase);
        }
        updatePlots();
    }
}

void CaseManager::updatePlotsAt(int dataIdx)
{
    currentDataIdx = dataIdx;
    validateCurrentDataIdx();
    updateSingleCaseDataIfShown(*simulationCases.begin()->second, dataIdx);
    for (auto& oneCase : simulationCases) {
        auto& simulationCase = oneCase.second;
        updateSingleCaseDataIfShown(*simulationCase, dataIdx);
    }
    for (auto& dataTimePlot : dataTimePlots) {
        dataTimePlot->update();
    }
    for (auto& distributionPlot : distributionPlots) {
        distributionPlot->update();
    }
}

void CaseManager::updatePlots()
{
    updatePlotsAt(currentDataIdx);
}

void CaseManager::hideAllCases()
{
    for (auto& oneCase : simulationCases) {
        setVisibility(oneCase.first, false);
    }
}

void CaseManager::setupBundles(const AbstractSimulationCase& simulationCase)
{
    for (auto& dataTimePlot : dataTimePlots) {
        dataTimePlot->provideBundle(simulationCase.caseName)->setColor(simulationCase.color);
    }
    for (auto& distributionPlot : distributionPlots) {
        distributionPlot->provideBundle(simulationCase.caseName)->setColor(simulationCase.color);
    }
}

void CaseManager::dropBundles(const AbstractSimulationCase &simulationCase)
{
    for (auto& dataTimePlot : dataTimePlots) {
        dataTimePlot->dropBundle(simulationCase.caseName);
    }
    for (auto& distributionPlot : distributionPlots) {
        distributionPlot->dropBundle(simulationCase.caseName);
    }
}

void CaseManager::updateSingleCaseDataIfShown(const AbstractSimulationCase &simulationCase, int dataIdx)
{
    if (simulationCase.isShown) {
        auto const& simulation = simulationCase.getSimulation();
        auto const& history = simulation.history;
        if (dataIdx >= history.moments.size()) {
            dataIdx = history.moments.size() - 1;
        }
        auto caseName = simulationCase.caseName;

        plotSumUtility->provideBundle(caseName)->updateData(history.sumUtilities, dataIdx);
        plotWealthDeviation->provideBundle(caseName)->updateData(history.wealthDeviation, dataIdx);

        plotQ1Traded->provideBundle(caseName)->updateData(history.q1Traded, dataIdx, simulation.getSumQ1());
        plotQ2Traded->provideBundle(caseName)->updateData(history.q2Traded, dataIdx, simulation.getSumQ2());
        plotNumSuccessfulTrades->provideBundle(caseName)->updateData(history.numSuccessful, dataIdx, simulation.getNumMaxTrade());

        auto const& moment = history.moments[dataIdx];
        plotQ1Distribution->provideBundle(caseName)->updateData(moment.q1Distribution);
        plotQ2Distribution->provideBundle(caseName)->updateData(moment.q2Distribution);
        plotUtilityDistribution->provideBundle(caseName)->updateData(moment.utilityDistribution);
        plotWealthDistribution->provideBundle(caseName)->updateData(moment.wealthDistribution);
    }
}

void CaseManager::validateCurrentDataIdx()
{
    auto const lastVisibleIdx = calculateLastVisibleDataIdx();
    if (currentDataIdx > lastVisibleIdx) {
        currentDataIdx = lastVisibleIdx;
    }
}
