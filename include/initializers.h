#pragma once

#include "data_point.h"

#include <cstddef>
#include <memory>
#include <random>
#include <string>
#include <vector>

class DistanceMetric;

class CentroidInitializer {
public:
    virtual ~CentroidInitializer() = default;

    virtual std::vector<DataPoint> initialize(
        const std::vector<DataPoint>& data,
        std::size_t clusterCount,
        std::mt19937& generator,
        const DistanceMetric& metric) const = 0;
};

class RandomInitializer final : public CentroidInitializer {
public:
    std::vector<DataPoint> initialize(
        const std::vector<DataPoint>& data,
        std::size_t clusterCount,
        std::mt19937& generator,
        const DistanceMetric& metric) const override;
};

class KMeansPlusPlusInitializer final : public CentroidInitializer {
public:
    std::vector<DataPoint> initialize(
        const std::vector<DataPoint>& data,
        std::size_t clusterCount,
        std::mt19937& generator,
        const DistanceMetric& metric) const override;
};

std::unique_ptr<CentroidInitializer> makeInitializer(const std::string& name);
