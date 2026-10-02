#include "distance_metric.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

double EuclideanMetric::operator()(const DataPoint& first, const DataPoint& second) const {
    return euclideanDistance(first, second);
}

std::string EuclideanMetric::name() const { return "euclidean"; }

double ManhattanMetric::operator()(const DataPoint& first, const DataPoint& second) const {
    return std::abs(first.x - second.x) + std::abs(first.y - second.y);
}

std::string ManhattanMetric::name() const { return "manhattan"; }

double ChebyshevMetric::operator()(const DataPoint& first, const DataPoint& second) const {
    return std::max(std::abs(first.x - second.x), std::abs(first.y - second.y));
}

std::string ChebyshevMetric::name() const { return "chebyshev"; }

std::unique_ptr<DistanceMetric> makeMetric(const std::string& name) {
    if (name == "euclidean") return std::make_unique<EuclideanMetric>();
    if (name == "manhattan") return std::make_unique<ManhattanMetric>();
    if (name == "chebyshev") return std::make_unique<ChebyshevMetric>();
    throw std::invalid_argument("unknown distance metric: " + name);
}
