#ifndef DATATIMERATIOPLOT_H
#define DATATIMERATIOPLOT_H

#include "datatimeplot.h"
#include "datatimeratioplottablebundle.h"

class DataTimeRatioPlot : public DataTimePlot
{
public:
    DataTimeRatioPlot(QCustomPlot* plot, QLabel *label, QString yLabel, QString labelPrefix);
    virtual void update() override;

    //outside users may use the pointer only while the plot is alive
    virtual DataTimeRatioPlottableBundle* provideBundle(QString bundleKey) override;

private:
    void setPercentageVisibility(bool visible);
    bool areMaxValuesTheSame() const;
    Amount_t castAndGetMaxValue(const DataTimePlottableBundle &baseBundle) const;
};

#endif // DATATIMERATIOPLOT_H
