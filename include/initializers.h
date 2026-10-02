#pragma once

#include "data_point.h"

#include <cstddef>
#include <random>
#include <vector>

class CentroidInitializer {
public:
    virtual ~CentroidInitializer() = default;

    virtual std::vector<DataPoint> initialize(
        const std::vector<DataPoint>& data,
        std::size_t clusterCount,
        std::mt19937& generator) const = 0;
};

class RandomInitializer final : public CentroidInitializer {
public:
    std::vector<DataPoint> initialize(
        const std::vector<DataPoint>& data,
        std::size_t clusterCount,
        std::mt19937& generator) const override;
};

class KMeansPlusPlusInitializer final : public CentroidInitializer {
public:
    std::vector<DataPoint> initialize(
        const std::vector<DataPoint>& data,
        std::size_t clusterCount,
        std::mt19937& generator) const override;
};
