#pragma once

#include "data_point.h"

#include <cstddef>
#include <random>
#include <vector>

using namespace std;

class CentroidInitializer {
public:
    virtual ~CentroidInitializer() = default;

    virtual vector<DataPoint> initialize(
        const vector<DataPoint>& data,
        size_t clusterCount,
        mt19937& generator) const = 0;
};

class RandomInitializer final : public CentroidInitializer {
public:
    vector<DataPoint> initialize(
        const vector<DataPoint>& data,
        size_t clusterCount,
        mt19937& generator) const override;
};

class KMeansPlusPlusInitializer final : public CentroidInitializer {
public:
    vector<DataPoint> initialize(
        const vector<DataPoint>& data,
        size_t clusterCount,
        mt19937& generator) const override;
};
