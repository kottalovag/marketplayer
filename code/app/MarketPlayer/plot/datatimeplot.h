#ifndef DATATIMEPLOT_H
#define DATATIMEPLOT_H

#include <QString>
#include <QLabel>
#include "modelutils.h"
#include "plot.h"

class DataTimePlot : public Plot
{
protected:
    QLabel* label;
    QString labelPrefix;
public:
    DataTimePlot(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix);
    void plotData(DataTimePair const& dataTime, int currentIdx);
    void reset();
};

#endif // DATATIMEPLOT_H
