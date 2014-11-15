#include "distributionplot.h"
#include "plotutils.h"

DistributionPlot::DistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel)
    : Plot(plot)
{
    auto bars = new QCPBars(plot->xAxis, plot->yAxis);
    plot->addPlottable(bars);
    plot->xAxis->setLabel(xLabel);
    plot->yAxis->setLabel(yLabel);
}

void DistributionPlot::plotData(const HeavyDistribution &distribution)
{
    auto const& data = distribution.data;

    auto bars = static_cast<QCPBars*>(plot->plottable(0));
    bars->setWidth(distribution.resolution);
    bars->setData(data.x, data.y);

    plot->xAxis->setRange(0.0, data.x.last() + distribution.resolution);
    plot->yAxis->setRange(0.0, distribution.maxNum * 1.1);

    plot->replot();
}
