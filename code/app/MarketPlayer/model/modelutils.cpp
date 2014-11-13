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

bool isPointInTriangle(const Position& p0, const Position& p1, const Position& p2, const Position& px) {
    //Barycentric method

    auto const& x1 = p0.q1;
    auto const& y1 = p0.q2;
    auto const& x2 = p1.q1;
    auto const& y2 = p1.q2;
    auto const& x3 = p2.q1;
    auto const& y3 = p2.q2;
    auto const& x = px.q1;
    auto const& y = px.q2;

    double denominator = ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
    double a = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / denominator;
    double b = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / denominator;
    double c = 1.0 - a - b;
    return  0.0 <= a && a <= 1.0 &&
            0.0 <= b && b <= 1.0 &&
            0.0 <= c && c <= 1.0;
}
