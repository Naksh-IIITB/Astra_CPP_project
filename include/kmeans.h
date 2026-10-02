#pragma once

#include "data_point.h"
#include "initializers.h"

#include <cstddef>
#include <memory>
#include <random>
#include <vector>

struct Cluster {
    DataPoint centroid;
    std::vector<std::size_t> memberIndices;
};

class KMeans {
public:
    KMeans(std::size_t clusterCount,
           std::unique_ptr<CentroidInitializer> initializer,
           std::size_t maxIterations = 100,
           double tolerance = 1e-4,
           unsigned int seed = std::random_device{}());

    void fit(const std::vector<DataPoint>& data);

    const std::vector<Cluster>& clusters() const noexcept;
    std::size_t iterations() const noexcept;
    bool converged() const noexcept;
    double inertia() const noexcept;

    void printResults() const;

private:
    void assignPoints(const std::vector<DataPoint>& data);
    bool updateCentroids(const std::vector<DataPoint>& data);

    std::size_t clusterCount_;
    std::unique_ptr<CentroidInitializer> initializer_;
    std::size_t maxIterations_;
    double tolerance_;
    std::mt19937 generator_;
    std::vector<Cluster> clusters_;
    std::size_t iterations_{};
    bool converged_{};
    double inertia_{};
};
