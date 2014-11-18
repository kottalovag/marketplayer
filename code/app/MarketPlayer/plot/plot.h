#ifndef PLOT_H
#define PLOT_H

#include "qcustomplot.h"

class Plot
{
protected:
    QCustomPlot* plot;
public:
    Plot(QCustomPlot* plot);
    virtual ~Plot() {}
    virtual void clearData();
};

#endif // PLOT_H
