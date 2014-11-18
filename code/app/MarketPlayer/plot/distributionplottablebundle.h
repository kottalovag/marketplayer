#ifndef DISTRIBUTIONPLOTTABLEBUNDLE_H
#define DISTRIBUTIONPLOTTABLEBUNDLE_H

#include "plottablebundle.h"
#include "modelutils.h"

class DistributionPlottableBundle : public PlottableBundle
{
public:
    DistributionPlottableBundle(QCustomPlot* plot);
    virtual void removeSelf() override;
    void updateData(HeavyDistribution const& distribution);
    Amount_t getXLast() const;
    Amount_t getMaxNum() const;

private:
    QCPBars* bars;
    Amount_t xLast;
    Amount_t maxNum;
};

#endif // DISTRIBUTIONPLOTTABLEBUNDLE_H
