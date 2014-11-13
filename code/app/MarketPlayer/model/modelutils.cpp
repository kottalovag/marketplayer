#include "modelutils.h"

bool ResourceToleranceEquality::operator()(const Amount_t& x, const Amount_t& y) const {
    return fabs(x-y) < std::numeric_limits<Amount_t>::epsilon();
}

ResourceDataPair sampleFunction(std::function<double (double)> func, Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution){
    ResourceDataPair dataPair;
    for (auto x = rangeStart; x <= rangeFinish; x += resolution) {
        dataPair.push(x, func(x));
    }
    return dataPair;
}

Distribution::Distribution(const vector<Amount_t>& subject, Amount_t resolution)
    : subject(subject)
    , resolution(resolution)
    , maxSubject(*std::max_element(subject.begin(), subject.end()))
    , numBuckets(static_cast<size_t>(ceil(maxSubject/resolution)))
{
    data.resize(numBuckets);

    //optimization possibility:
    Amount_t currentBucket = resolution/2;
    for (size_t idx = 0; idx < numBuckets; ++idx, currentBucket += resolution) {
        data.x[idx] = currentBucket;
    }

    for (auto const& amount : subject) {
        auto bucketIdx = static_cast<ResourceDataPair::size_type>(floor(amount/resolution));
        data.y[bucketIdx] += 1;
    }
}

void HeavyDistribution::setup(const vector<Amount_t> &subject, Amount_t resolution)
{
    Distribution distribution(subject, resolution);
    this->data = std::move(distribution.data);
    this->resolution = distribution.resolution;
    this->maxSubject = distribution.maxSubject;
    this->maxNum = *std::max_element(data.y.begin(), data.y.end());
    this->numBuckets = distribution.numBuckets;
    this->standardDeviation = calculateStandardDeviation(subject);
}
