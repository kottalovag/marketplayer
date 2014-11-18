#ifndef BUNDLEHELPER_H
#define BUNDLEHELPER_H

#include <map>
#include <QString>
#include <memory>
#include <functional>

#include "qcustomplot.h"
#include "modelutils.h"

namespace BundleHelper {

template<class BundleType, class MappedBundleType = BundleType>
BundleType* provideBundleHelper(QString bundleKey, std::map<QString, std::unique_ptr<MappedBundleType>>& bundles, QCustomPlot* plot)
{
    if (bundles.find(bundleKey) == bundles.end()) {
        std::unique_ptr<BundleType> bundle(new BundleType(plot));
        bundles.emplace(bundleKey, std::move(bundle));
    }
    return static_cast<BundleType*>(bundles[bundleKey].get());
}

template<class BundleType>
void dropBundleHelper(QString bundleKey, std::map<QString, std::unique_ptr<BundleType>>& bundles)
{
    auto foundElement = bundles.find(bundleKey);
    if (foundElement != bundles.end()) {
        auto& bundlePtr = foundElement->second;
        bundlePtr->removeSelf();
        bundles.erase(foundElement);
    }
}

template<class BundleType>
Amount_t bundleMax(std::map<QString, std::unique_ptr<BundleType>>& bundles,
                   std::function<Amount_t(std::unique_ptr<BundleType> const&)> getter)
{
    typedef std::map<QString, std::unique_ptr<BundleType>> BundleMap;
    typedef typename BundleMap::value_type OnePair;
    auto const maxElement = std::max_element(bundles.begin(), bundles.end(),
        [&](OnePair const& a, OnePair const& b){
            return getter(a.second) < getter(b.second);
        }
    );
    return getter(maxElement->second);
}

template<class PairType, class ExaminedType, class ResultType>
bool compareBundles(PairType const& a, PairType const& b, std::function<ResultType(ExaminedType const&)> getter)
{
    return getter(a->second) < getter(b->second);
}

}

#endif // BUNDLEHELPER_H
