#include "plottablebundle.h"
#include "plotutils.h"

PlottableBundle::PlottableBundle()
    :color(QColor("blue"))
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

void PlottableBundle::applyColor(PlottableInfo& info)
{
    if (info.second == DynamicColor) {
        auto currentPen = info.first->pen();
        currentPen.setColor(color);
        info.first->setPen(currentPen);
    }
}
