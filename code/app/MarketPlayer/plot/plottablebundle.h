#ifndef PLOTTABLEBUNDLE_H
#define PLOTTABLEBUNDLE_H

#include "qcustomplot.h"
#include <QList>
#include <QColor>

struct PlottableBundle
{
    enum ColorStrategy {
        FixColor,
        DynamicColor
    };

    PlottableBundle();
    void setColor(QColor color);
    void clearData();
    void addPlottable(QCPAbstractPlottable* plottable, ColorStrategy colorStrategy = DynamicColor);
private:
    typedef QPair<QCPAbstractPlottable*, ColorStrategy> PlottableInfo;

    void applyColor(PlottableInfo &info);

    QColor color;
    QList<PlottableInfo> plottables;
};

#endif // PLOTTABLEBUNDLE_H
