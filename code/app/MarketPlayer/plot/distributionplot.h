#ifndef DISTRIBUTIONPLOT_H
#define DISTRIBUTIONPLOT_H

#include <QString>
#include <memory>

#include "modelutils.h"
#include "plot.h"
#include "distributionplottablebundle.h"

class DistributionPlot : public Plot
{
public:
    DistributionPlot(QCustomPlot* plot, QString xLabel, QString yLabel);
    void update();

    //outside users may use the pointer only while the plot is alive
    DistributionPlottableBundle* provideBundle(QString bundleKey);
    void dropBundle(QString bundleKey);

protected:
    //DataTimePlottableBundle must not outlive the plot
    typedef std::map<QString, std::unique_ptr<DistributionPlottableBundle>> BundleMap;
    BundleMap bundles;
};

#endif // DISTRIBUTIONPLOT_H
