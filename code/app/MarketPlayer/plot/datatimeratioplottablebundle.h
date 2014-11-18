#ifndef DATATIMERATIOPLOTTABLEBUNDLE_H
#define DATATIMERATIOPLOTTABLEBUNDLE_H

#include "datatimeplottablebundle.h"

struct DataTimeRatioPlottableBundle : public DataTimePlottableBundle
{
    DataTimeRatioPlottableBundle(QCustomPlot* plot);
    void updateData(DataTimePair const& dataTime, int currentIdx, Amount_t maxValue);
    Amount_t getMaxValue() const;

private:
    Amount_t maxValue;
};

#endif // DATATIMERATIOPLOTTABLEBUNDLE_H
