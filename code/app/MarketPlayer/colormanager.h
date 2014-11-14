#ifndef COLORMANAGER_H
#define COLORMANAGER_H

#include <QVector>
#include <QColor>

struct ColorManager
{
    ColorManager();
    QColor provideNextColor();
    void reset();

private:
    size_t currentIdx;
    QVector<QColor> colors;
};

#endif // COLORMANAGER_H
