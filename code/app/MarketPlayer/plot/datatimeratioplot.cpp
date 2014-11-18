#include "datatimeratioplot.h"
#include "bundlehelper.h"

DataTimeRatioPlot::DataTimeRatioPlot(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix)
    : DataTimePlot(plot, label, yLabel, labelPrefix)
{
    setPercentageVisibility(false);
}

void DataTimeRatioPlot::update()
{
    bool const canShowPercentage = areMaxValuesTheSame();
    setPercentageVisibility(canShowPercentage);
    if (canShowPercentage) {
        auto const& bundle = *bundles.begin()->second;
        auto const maxValue = castAndGetMaxValue(bundle);
        auto const yMax = bundle.getYMax();
        Amount_t const percentageRangeMax = yMax / maxValue * 100;
        plot->yAxis2->setRange(0.0, percentageRangeMax * 1.1);
    }
    DataTimePlot::update();
}

DataTimeRatioPlottableBundle *DataTimeRatioPlot::provideBundle(QString bundleKey)
{
    return BundleHelper::provideBundleHelper
            <DataTimeRatioPlottableBundle, DataTimePlottableBundle>
            (bundleKey, bundles, plot);
}

void DataTimeRatioPlot::setPercentageVisibility(bool visible)
{
    plot->yAxis2->setVisible(visible);
    plot->yAxis2->setLabel(visible ? "%" : "");
}

bool DataTimeRatioPlot::areMaxValuesTheSame() const
{
    if (bundles.size() == 0) {
        return false;
    } else if (bundles.size() > 1) {
        auto const firstMaxValue = castAndGetMaxValue(*bundles.begin()->second);
        for (auto& bundlePair : bundles) {
            auto const otherMaxValue = castAndGetMaxValue(*bundlePair.second);
            if (otherMaxValue != firstMaxValue) {
                return false;
            }
        }
    }
    return true;
}

Amount_t DataTimeRatioPlot::castAndGetMaxValue(DataTimePlottableBundle const& baseBundle) const
{
    auto const bundle = static_cast<DataTimeRatioPlottableBundle const&>(baseBundle);
    return bundle.getMaxValue();
}
