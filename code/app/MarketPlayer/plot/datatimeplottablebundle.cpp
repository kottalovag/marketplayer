#include "datatimeplottablebundle.h"

DataTimePlottableBundle::DataTimePlottableBundle(QCustomPlot *plot)
    : PlottableBundle(plot)
{
    dataGraph = plot->addGraph();
    addPlottable(dataGraph, DynamicColor);

    currentPointGraph = plot->addGraph();
    addPlottable(currentPointGraph, FixColor);
    currentPointGraph->setLineStyle(QCPGraph::lsNone);
    currentPointGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross));
    auto currentPen = currentPointGraph->pen();
    currentPen.setColor(Qt::black);
    currentPen.setWidth(2);
    currentPointGraph->setPen(currentPen);
}

void DataTimePlottableBundle::removeSelf()
{
    PlottableBundle::removeSelf();
    dataGraph = nullptr;
    currentPointGraph = nullptr;
}

void DataTimePlottableBundle::updateData(const DataTimePair &dataTime, int currentIdx)
{
    dataGraph->setData(dataTime.data.x, dataTime.data.y);

    //if out of range:
    if (currentIdx >= dataTime.size()) {
        currentIdx = dataTime.size() - 1;
    }
    currentPointGraph->clearData();
    currentPointGraph->addData(currentIdx, dataTime[currentIdx]);

    xLast = dataTime.data.x.last();
    yMax = dataTime.max;
    currentValue = dataTime[currentIdx];
}

Amount_t DataTimePlottableBundle::getXLast() const
{
    return xLast;
}

Amount_t DataTimePlottableBundle::getYMax() const
{
    return yMax;
}

Amount_t DataTimePlottableBundle::getCurrentValue() const
{
    return currentValue;
}
