#pragma once

#include "kmeans.h"
#include "distance_metric.h"

#include <cstddef>
#include <vector>

using namespace std;

struct ClusterSummary {
    size_t index{};
    size_t size{};
    double averageDistance{};
    double maximumDistance{};
};

double silhouetteScore(const vector<DataPoint>& data, const vector<Cluster>& clusters,
                       const DistanceMetric& metric);
vector<ClusterSummary> summarizeClusters(const vector<DataPoint>& data,
                                        const vector<Cluster>& clusters,
                                        const DistanceMetric& metric);
vector<size_t> findOutliers(const vector<DataPoint>& data,
                            const vector<Cluster>& clusters,
                            const DistanceMetric& metric,
                            double zScoreThreshold = 2.5);
