#ifndef DATATIMEPLOT_H
#define DATATIMEPLOT_H

#include <QString>
#include <QLabel>
#include <map>
#include <memory>

#include "modelutils.h"
#include "plot.h"
#include "datatimeplottablebundle.h"

class DataTimePlot : public Plot
{
public:
    DataTimePlot(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix);
    virtual void update();
    virtual void clearData() override;

    //outside users may use the pointer only while the plot is alive
    virtual DataTimePlottableBundle* provideBundle(QString bundleKey);
    void dropBundle(QString bundleKey);

protected:
    QLabel* label;
    QString labelPrefix;

    //DataTimePlottableBundle must not outlive the plot
    typedef std::map<QString, std::unique_ptr<DataTimePlottableBundle>> BundleMap;
    BundleMap bundles;
};

#endif // DATATIMEPLOT_H
