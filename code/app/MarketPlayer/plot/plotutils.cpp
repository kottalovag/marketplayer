#include "plotutils.h"
#include "qcustomplot.h"

void cleanPlotData(QCustomPlot* plot)
{
    for (size_t graphIdx = 0; graphIdx < plot->graphCount(); ++graphIdx) {
        plot->graph(graphIdx)->clearData();
    }
    for (size_t plottableIdx = 0; plottableIdx < plot->plottableCount(); ++plottableIdx) {
        plot->plottable(plottableIdx)->clearData();
    }
}
