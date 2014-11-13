#ifndef DISTRIBUTIONPLOT_H
#define DISTRIBUTIONPLOT_H

#include <QString>

#include "modelutils.h"
#include "plot.h"

class DistributionPlot : public Plot
{
public:
    DistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel);
    void plotData(HeavyDistribution const& distribution);
};

#endif // DISTRIBUTIONPLOT_H
