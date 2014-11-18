#ifndef DATATIMEPLOTTABLEBUNDLE_H
#define DATATIMEPLOTTABLEBUNDLE_H

#include "plottablebundle.h"
#include "modelutils.h"

class QCustomPlot;

struct DataTimePlottableBundle : public PlottableBundle
{
    DataTimePlottableBundle(QCustomPlot* plot);
    virtual void removeSelf() override;
    void updateData(DataTimePair const& dataTime, int currentIdx);
    Amount_t getXLast() const;
    Amount_t getYMax() const;
    Amount_t getCurrentValue() const;

private:
    QCPGraph* dataGraph;
    QCPGraph* currentPointGraph;
    Amount_t xLast;
    Amount_t yMax;
    Amount_t currentValue;
};

#endif // DATATIMEPLOTTABLEBUNDLE_H
