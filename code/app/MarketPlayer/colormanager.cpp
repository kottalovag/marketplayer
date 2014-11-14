#include "colormanager.h"

ColorManager::ColorManager()
{
    reset();
    for (auto colorName : {"blue", "red", "green", "black", "orange", "darkMagenta"}) {
        colors.push_back(QColor(colorName));
    }
}

QColor ColorManager::provideNextColor()
{
    if (colors.size() == 0) {
        return QColor("black");
    } else {
        auto result = colors[currentIdx];
        currentIdx = (currentIdx+1)%colors.size();
        return result;
    }
}

void ColorManager::reset()
{
    currentIdx = 0;
}
