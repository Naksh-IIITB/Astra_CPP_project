#pragma once

#include "data_point.h"
#include "initializers.h"

#include <cstddef>
#include <random>
#include <vector>

using namespace std;

struct Cluster {
    DataPoint centroid;
    vector<size_t> memberIndices;
};

class KMeans {
public:
    KMeans(size_t clusterCount,
           const CentroidInitializer& initializer,
           size_t maxIterations = 100,
           double tolerance = 1e-4,
           unsigned int seed = random_device{}());

    void fit(const vector<DataPoint>& data);

    const vector<Cluster>& clusters() const noexcept;
    size_t iterations() const noexcept;
    bool converged() const noexcept;
    double inertia() const noexcept;

    void printResults() const;

private:
    void assignPoints(const vector<DataPoint>& data);
    bool updateCentroids(const vector<DataPoint>& data);

    size_t clusterCount_;
    const CentroidInitializer& initializer_;
    size_t maxIterations_;
    double tolerance_;
    mt19937 generator_;
    vector<Cluster> clusters_;
    size_t iterations_{};
    bool converged_{};
    double inertia_{};
};
