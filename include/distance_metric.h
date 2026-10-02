#pragma once

#include "data_point.h"

#include <memory>
#include <string>

class DistanceMetric {
public:
    virtual ~DistanceMetric() = default;
    virtual double operator()(const DataPoint& first, const DataPoint& second) const = 0;
    virtual std::string name() const = 0;
};

class EuclideanMetric final : public DistanceMetric {
public:
    double operator()(const DataPoint& first, const DataPoint& second) const override;
    std::string name() const override;
};

class ManhattanMetric final : public DistanceMetric {
public:
    double operator()(const DataPoint& first, const DataPoint& second) const override;
    std::string name() const override;
};

class ChebyshevMetric final : public DistanceMetric {
public:
    double operator()(const DataPoint& first, const DataPoint& second) const override;
    std::string name() const override;
};

std::unique_ptr<DistanceMetric> makeMetric(const std::string& name);
