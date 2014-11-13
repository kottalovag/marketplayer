#include "datatimeplot.h"
#include "plotutils.h"

DataTimePlot::DataTimePlot(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix)
    : Plot(plot)
    , label(label)
    , labelPrefix(labelPrefix)
{
    plot->xAxis->setLabel("Time");
    plot->yAxis->setLabel(yLabel);

    //data
    plot->addGraph();

    //current point
    plot->addGraph();
    auto currentGraph = plot->graph(1);
    currentGraph->setLineStyle(QCPGraph::lsNone);
    currentGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross));
    auto currentPen = currentGraph->pen();
    currentPen.setColor(Qt::black);
    currentPen.setWidth(2);
    currentGraph->setPen(currentPen);
}

void DataTimePlot::plotData(const DataTimePair &dataTime, int currentIdx)
{
    plot->xAxis->setRange(0.0, dataTime.data.x.last() + 1);
    plot->yAxis->setRange(0.0, dataTime.max * 1.1);

    plot->graph(0)->setData(dataTime.data.x, dataTime.data.y);

    plot->graph(1)->clearData();
    plot->graph(1)->addData(currentIdx, dataTime[currentIdx]);
    plot->replot();

    label->setText(labelPrefix + " :" + QString::number(dataTime[currentIdx]));
}

void DataTimePlot::reset()
{
    Plot::reset();
    if (label) {
        label->setText(labelPrefix + ": 0");
    }
}
