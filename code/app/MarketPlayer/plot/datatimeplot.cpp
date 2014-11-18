#include "datatimeplot.h"
#include "plotutils.h"
#include "bundlehelper.h"

using std::unique_ptr;
using std::map;

DataTimePlot::DataTimePlot(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix)
    : Plot(plot)
    , label(label)
    , labelPrefix(labelPrefix)
{
    plot->xAxis->setLabel("Time");
    plot->yAxis->setLabel(yLabel);
}

void DataTimePlot::update()
{
    if (bundles.size() > 0) {
        typedef std::unique_ptr<DataTimePlottableBundle> BundlePtr;
        auto const xLast = BundleHelper::bundleMax<DataTimePlottableBundle>(
                    bundles,
                    [](BundlePtr const& bundle){ return bundle->getXLast(); }
        );
        auto const yMax = BundleHelper::bundleMax<DataTimePlottableBundle>(
                    bundles,
                    [](BundlePtr const& bundle){ return bundle->getYMax(); }
        );

        plot->xAxis->setRange(0.0, xLast + 1);
        plot->yAxis->setRange(0.0, yMax * 1.1);
    }

    plot->replot();

    if (bundles.size() == 1) {
        label->setText(labelPrefix + " :" + QString::number(bundles.begin()->second->getCurrentValue()));
    } else {
        label->setText("");
    }
}

void DataTimePlot::clearData()
{
    Plot::clearData();
    if (label) {
        label->setText("");
    }
}

DataTimePlottableBundle *DataTimePlot::provideBundle(QString bundleKey)
{
    return BundleHelper::provideBundleHelper<DataTimePlottableBundle>(bundleKey, bundles, plot);
}

void DataTimePlot::dropBundle(QString bundleKey)
{
    BundleHelper::dropBundleHelper<DataTimePlottableBundle>(bundleKey, bundles);
}
