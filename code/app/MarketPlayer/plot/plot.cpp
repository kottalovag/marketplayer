#include "plot.h"
#include "plotutils.h"

Plot::Plot(QCustomPlot* plot)
    :plot(plot)
{
}

void Plot::reset()
{
    if (plot) {
        cleanPlotData(plot);
    }
}
