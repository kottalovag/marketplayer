#include "distributionplot.h"
#include "plotutils.h"
#include "bundlehelper.h"

DistributionPlot::DistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel)
    : Plot(plot)
{
    plot->xAxis->setLabel(xLabel);
    plot->yAxis->setLabel(yLabel);
}

void DistributionPlot::update()
{
    if (bundles.size() > 0) {
        typedef std::unique_ptr<DistributionPlottableBundle> BundlePtr;
        auto const xLast = BundleHelper::bundleMax<DistributionPlottableBundle>(
                    bundles,
                    [](BundlePtr const& bundle){ return bundle->getXLast(); }
        );
        auto const maxNum = BundleHelper::bundleMax<DistributionPlottableBundle>(
                    bundles,
                    [](BundlePtr const& bundle){ return bundle->getMaxNum(); }
        );

        plot->xAxis->setRange(0.0, xLast * 1.1);
        plot->yAxis->setRange(0.0, maxNum * 1.1);
    }

    plot->replot();
}

DistributionPlottableBundle *DistributionPlot::provideBundle(QString bundleKey)
{
    return BundleHelper::provideBundleHelper<DistributionPlottableBundle>(bundleKey, bundles, plot);
}

void DistributionPlot::dropBundle(QString bundleKey)
{
    BundleHelper::dropBundleHelper<DistributionPlottableBundle>(bundleKey, bundles);
}
