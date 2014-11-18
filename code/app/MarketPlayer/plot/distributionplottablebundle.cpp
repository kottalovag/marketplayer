#include "distributionplottablebundle.h"

DistributionPlottableBundle::DistributionPlottableBundle(QCustomPlot *plot)
    : PlottableBundle(plot)
{
    bars = new QCPBars(plot->xAxis, plot->yAxis);
    plot->addPlottable(bars);
    addPlottable(bars, DynamicColor);
}

void DistributionPlottableBundle::removeSelf()
{
    PlottableBundle::removeSelf();
    bars = nullptr;
}

void DistributionPlottableBundle::updateData(const HeavyDistribution &distribution)
{
    auto const& data = distribution.data;

    bars->setWidth(distribution.resolution);
    bars->setData(data.x, data.y);

    xLast = data.x.last();
    maxNum = distribution.maxNum;
}

Amount_t DistributionPlottableBundle::getXLast() const
{
    return xLast;
}

Amount_t DistributionPlottableBundle::getMaxNum() const
{
    return maxNum;
}
