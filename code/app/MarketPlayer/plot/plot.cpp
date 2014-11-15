#include "plot.h"
#include "plotutils.h"

Plot::Plot(QCustomPlot* plot)
    :plot(plot)
{
}

void Plot::clearData()
{
    if (plot) {
        clearPlotData(plot);
    }
}
