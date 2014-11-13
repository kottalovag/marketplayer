#ifndef DATATIMEPLOTWITHPERCENTAGE_H
#define DATATIMEPLOTWITHPERCENTAGE_H

#include "datatimeplot.h"

class DataTimePlotWithPercentage : public DataTimePlot
{
public:
    DataTimePlotWithPercentage(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix);
    void plotDataAndPercentage(DataTimePair const& dataTime, int currentIdx, Amount_t max);
    void plotData(const DataTimePair &dataTime, int currentIdx) = delete;
};

#endif // DATATIMEPLOTWITHPERCENTAGE_H
