#include "datatimeratioplottablebundle.h"

DataTimeRatioPlottableBundle::DataTimeRatioPlottableBundle(QCustomPlot *plot)
    : DataTimePlottableBundle(plot)
{
}

void DataTimeRatioPlottableBundle::updateData(const DataTimePair &dataTime, int currentIdx, double maxValue)
{
    DataTimePlottableBundle::updateData(dataTime, currentIdx);
    this->maxValue = maxValue;
}

Amount_t DataTimeRatioPlottableBundle::getMaxValue() const
{
    return maxValue;
}
