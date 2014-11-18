#ifndef CASEMANAGER_H
#define CASEMANAGER_H

#include <map>
#include <QString>
#include <QColor>
#include <vector>

#include "model.h"
#include "simulationcase.h"
#include "datatimeplot.h"
#include "datatimeratioplot.h"
#include "distributionplot.h"

struct CaseManager
{
    CaseManager(
            DataTimeRatioPlot* plotQ1Traded,
            DataTimeRatioPlot* plotQ2Traded,
            DataTimePlot* plotSumUtility,
            DataTimeRatioPlot* plotNumSuccessfulTrades,
            DataTimePlot* plotWealthDeviation,
            DistributionPlot* plotQ1Distribution,
            DistributionPlot* plotQ2Distribution,
            DistributionPlot* plotUtilityDistribution,
            DistributionPlot* plotWealthDistribution
                );
    bool contains(QString caseName) const;
    const AbstractSimulationCase &getSimulationCase(QString caseName) const;
    size_t calculateLastVisibleDataIdx() const;
    size_t getValidCurrentDataIdx() const;

    void addHeavyCase(QString caseName, Simulation const& simulation, QColor color, bool visible);
    void addExternalCase(QString caseName, Simulation const& simulation, QColor color, bool visible);
    void removeCase(QString caseName);
    void setVisibility(QString caseName, bool visible);
    void updatePlotsAt(int dataIdx);
    void updatePlots();
    void hideAllCases();

private:
    template<class CaseType>
    void addCase(Simulation const& simulation, QString caseName, QColor color, bool visible) {
        if (!contains(caseName)) {
            auto emplaceResult = simulationCases.emplace(caseName, unique_ptr<CaseType>(
                                        new CaseType(simulation, caseName, color, visible)));
            auto const& emplacedPair = *emplaceResult.first;
            auto const& emplacedCase = *emplacedPair.second;
            if (visible) {
                setupBundles(emplacedCase);
            }
        } //else omgWTF
    }

    void setupBundles(const AbstractSimulationCase& simulationCase);
    void dropBundles(const AbstractSimulationCase& simulationCase);
    void updateSingleCaseDataIfShown(AbstractSimulationCase const& simulationCase, int dataIdx);
    void validateCurrentDataIdx();

    std::map<QString, unique_ptr<AbstractSimulationCase>> simulationCases;

    DataTimeRatioPlot* plotQ1Traded;
    DataTimeRatioPlot* plotQ2Traded;
    DataTimePlot* plotSumUtility;
    DataTimeRatioPlot* plotNumSuccessfulTrades;
    DataTimePlot* plotWealthDeviation;

    DistributionPlot* plotQ1Distribution;
    DistributionPlot* plotQ2Distribution;
    DistributionPlot* plotUtilityDistribution;
    DistributionPlot* plotWealthDistribution;

    std::vector<DataTimePlot*> dataTimePlots;
    std::vector<DistributionPlot*> distributionPlots;
    std::vector<Plot*> plots;

    size_t currentDataIdx;
};

#endif // CASEMANAGER_H
