#ifndef MODELUTILS_H
#define MODELUTILS_H

#include <functional>
#include <QVector>

typedef double Amount_t;

template<typename E>
//using vector = std::vector<E>;
using vector = QVector<E>;

struct Position;
extern std::function<void(Position const&)> debugShowPoint;

typedef std::mt19937 URNG;
extern URNG globalUrng;

struct Position
{
    Amount_t q1, q2;
    Position operator-(const Position& other) const {
        return Position{q1 - other.q1, q2 - other.q2};
    }
    Position operator+(const Position& other) const {
        return Position{q1 + other.q1, q2 + other.q2};
    }
    Position operator*(const double& factor) const {
        return Position{q1*factor, q2*factor};
    }
};

template<typename T>
struct DataPair
{
    typedef typename vector<T>::size_type size_type;
    vector<T> x,y;
    size_type size() const { return x.size(); }

    void push(T const& x, T const& y) {
        this->x.push_back(x);
        this->y.push_back(y);
    }

    void resize(size_type size) {
        x.resize(size);
        y.resize(size);
    }
    void reset() {
        resize(0);
    }
};

typedef DataPair<Amount_t> ResourceDataPair;

struct DataTimePair
{
    DataPair<Amount_t> data;
    Amount_t max;
    Amount_t getLastX() const { return data.x.size() - 1; }
    void push(Amount_t newData) {
        data.push(getLastX() + 1, newData);
        if (newData > max) {
            max = newData;
        }
    }
    void reset() {
        max = 0.0;
        data.reset();
    }
    Amount_t const& operator[](size_t idx) const {
        return data.y[idx];
    }
};

struct IndexNumber
{
    unsigned int actual;
    IndexNumber()
        :actual(0)
    {}
    unsigned int operator()() {return actual++;}
};

struct ResourceToleranceEquality
{
    bool operator()(Amount_t const& x, Amount_t const& y) const;
};

template<typename T>
T calculateStandardDeviation(vector<T> const& subject)
{
    if (subject.size() > 1) {
        auto const sum = std::accumulate(subject.begin(), subject.end(), 0.0);
        auto const mean = sum / subject.size();
        T accumulated = 0.0;
        for (auto v : subject) {
            accumulated += (v-mean) * (v-mean);
        }
        auto result = sqrt(accumulated / (subject.size()-1));
        return result;
    } else {
        return T{};
    }
}

ResourceDataPair sampleFunction(std::function<double(double)> func, Amount_t rangeStart, Amount_t rangeFinish, Amount_t resolution);

struct Distribution {
    vector<Amount_t> const& subject;
    Amount_t const resolution;
    Amount_t const maxSubject;
    size_t const numBuckets;

    ResourceDataPair data;

    Distribution(vector<Amount_t> const& subject, Amount_t resolution);
};

struct HeavyDistribution
{
    Amount_t resolution;
    Amount_t maxSubject;
    Amount_t maxNum;
    size_t numBuckets;
    ResourceDataPair data;
    Amount_t standardDeviation;

    void setup(vector<Amount_t> const& subject, Amount_t resolution);
};

bool isPointInTriangle(Position const& p0, Position const& p1, Position const& p2, Position const& px);

#endif // MODELUTILS_H
