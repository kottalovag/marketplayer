#include "plottablebundle.h"
#include "plotutils.h"

PlottableBundle::PlottableBundle(QCustomPlot *plot)
    : plot(plot)
    , color(QColor("blue"))
{
}

void PlottableBundle::setColor(QColor color)
{
    this->color = color;
    for (auto plottable : plottables) {
        applyColor(plottable);
    }
}

void PlottableBundle::clearData()
{
    for (auto plottable : plottables) {
        plottable.first->clearData();
    }
}

void PlottableBundle::addPlottable(QCPAbstractPlottable *plottable, ColorStrategy colorStrategy)
{
    PlottableInfo info{plottable, colorStrategy};
    plottables.append(info);
    applyColor(info);
}

//This must not be called after destruction of plot
//so it may not go into the destructor. (Lifetime could be changed)
//However we need this functionality to be able
//to remove bundles manually
void PlottableBundle::removeSelf()
{
    for (auto plottableInfo : plottables) {
        plot->removePlottable(plottableInfo.first);
    }
}

void PlottableBundle::applyColor(PlottableInfo& info)
{
    if (info.second == DynamicColor) {
        auto currentPen = info.first->pen();
        currentPen.setColor(color);
        info.first->setPen(currentPen);
    }
}
