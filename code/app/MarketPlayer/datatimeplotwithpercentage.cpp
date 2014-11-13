#include "datatimeplotwithpercentage.h"

DataTimePlotWithPercentage::DataTimePlotWithPercentage(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix)
    : DataTimePlot(plot, label, yLabel, labelPrefix)
{
    plot->yAxis2->setVisible(true);
    plot->yAxis2->setLabel("%");
}

void DataTimePlotWithPercentage::plotDataAndPercentage(const DataTimePair &dataTime, int currentIdx, Amount_t max)
{
    DataTimePlot::plotData(dataTime, currentIdx);
    Amount_t percentageRangeMax = dataTime.max / max * 100;
    plot->yAxis2->setRange(0.0, percentageRangeMax * 1.1);
}
