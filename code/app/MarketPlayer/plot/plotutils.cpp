#include "plotutils.h"
#include "qcustomplot.h"

void clearPlotData(QCustomPlot* plot)
{
    for (int plottableIdx = 0; plottableIdx < plot->plottableCount(); ++plottableIdx) {
        plot->plottable(plottableIdx)->clearData();
    }
}
