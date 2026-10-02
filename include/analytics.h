#pragma once

#include "distance_metric.h"
#include "kmeans.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class ClusterMetric {
public:
    virtual ~ClusterMetric() = default;
    virtual double compute(const std::vector<DataPoint>& data,
                           const std::vector<Cluster>& clusters) const = 0;
    virtual std::string name() const = 0;
};

class InertiaMetric final : public ClusterMetric {
public:
    double compute(const std::vector<DataPoint>& data,
                   const std::vector<Cluster>& clusters) const override;
    std::string name() const override;
};

class SilhouetteMetric final : public ClusterMetric {
public:
    explicit SilhouetteMetric(std::unique_ptr<DistanceMetric> metric = std::make_unique<EuclideanMetric>());
    double compute(const std::vector<DataPoint>& data,
                   const std::vector<Cluster>& clusters) const override;
    std::string name() const override;

private:
    std::unique_ptr<DistanceMetric> metric_;
};

struct ClusterSummary {
    std::size_t index{};
    std::size_t size{};
    double averageDistance{};
    double maximumDistance{};
};

class ClusterDiagnostics {
public:
    explicit ClusterDiagnostics(const DistanceMetric& metric) : metric_(metric) {}
    std::vector<ClusterSummary> summarize(const std::vector<DataPoint>& data,
                                          const std::vector<Cluster>& clusters) const;
    std::vector<std::size_t> findOutliers(const std::vector<DataPoint>& data,
                                          const std::vector<Cluster>& clusters,
                                          double zScoreThreshold = 2.5) const;

private:
    const DistanceMetric& metric_;
};
