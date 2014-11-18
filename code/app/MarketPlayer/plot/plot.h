#ifndef PLOT_H
#define PLOT_H

#include "qcustomplot.h"
#include "modelutils.h" //todo remove

class Plot
{
protected:
    QCustomPlot* plot;
public:
    Plot(QCustomPlot* plot);
    virtual ~Plot() {}
    virtual void clearData();

    //todo remove
    void plotData(HeavyDistribution const&) {}
    void plotData(DataTimePair const&, int){}
    void plotDataAndPercentage(DataTimePair const&, int, double) {}
};

#endif // PLOT_H
