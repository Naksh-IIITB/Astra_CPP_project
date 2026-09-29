#pragma once

#include "kmeans.h"

#include <cstddef>
#include <vector>

struct ClusterSummary {
    size_t index{};
    size_t size{};
    double averageDistance{};
    double maximumDistance{};
};

double silhouetteScore(const vector<DataPoint>& data, const vector<Cluster>& clusters);
vector<ClusterSummary> summarizeClusters(const vector<DataPoint>& data,
                                        const vector<Cluster>& clusters);
vector<size_t> findOutliers(const vector<DataPoint>& data,
                            const vector<Cluster>& clusters,
                            double zScoreThreshold = 2.5);
