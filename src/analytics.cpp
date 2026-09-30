#include "analytics.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

using namespace std;

namespace {
double averageDistanceToCluster(const DataPoint& point,
                                size_t ownIndex,
                                const vector<DataPoint>& data,
                                const Cluster& cluster) {
    if (cluster.memberIndices.empty()) {
        return 0.0;
    }
    double total = 0.0;
    size_t count = 0;
    for (size_t member : cluster.memberIndices) {
        if (member != ownIndex) {
            total += distance(point, data[member]);
            ++count;
        }
    }
    return count == 0 ? 0.0 : total / static_cast<double>(count);
}
}  // namespace

double silhouetteScore(const vector<DataPoint>& data, const vector<Cluster>& clusters) {
    if (data.size() < 2 || clusters.size() < 2) {
        return 0.0;
    }
    double total = 0.0;
    size_t evaluated = 0;
    for (size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        for (size_t pointIndex : clusters[clusterIndex].memberIndices) {
            const double intra = averageDistanceToCluster(
                data[pointIndex], pointIndex, data, clusters[clusterIndex]);
            double nearestOther = numeric_limits<double>::infinity();
            for (size_t other = 0; other < clusters.size(); ++other) {
                if (other != clusterIndex && !clusters[other].memberIndices.empty()) {
                    nearestOther = min(nearestOther, averageDistanceToCluster(
                        data[pointIndex], pointIndex, data, clusters[other]));
                }
            }
            const double denominator = max(intra, nearestOther);
            total += denominator == 0.0 ? 0.0 : (nearestOther - intra) / denominator;
            ++evaluated;
        }
    }
    return evaluated == 0 ? 0.0 : total / static_cast<double>(evaluated);
}

vector<ClusterSummary> summarizeClusters(const vector<DataPoint>& data,
                                        const vector<Cluster>& clusters) {
    vector<ClusterSummary> summaries;
    summaries.reserve(clusters.size());
    for (size_t index = 0; index < clusters.size(); ++index) {
        const Cluster& cluster = clusters[index];
        double totalDistance = 0.0;
        double maximumDistance = 0.0;
        for (size_t pointIndex : cluster.memberIndices) {
            const double memberDistance = distance(data[pointIndex], cluster.centroid);
            totalDistance += memberDistance;
            maximumDistance = max(maximumDistance, memberDistance);
        }
        const double average = cluster.memberIndices.empty()
            ? 0.0 : totalDistance / static_cast<double>(cluster.memberIndices.size());
        summaries.push_back({index, cluster.memberIndices.size(), average, maximumDistance});
    }
    return summaries;
}

vector<size_t> findOutliers(const vector<DataPoint>& data,
                            const vector<Cluster>& clusters,
                            double zScoreThreshold) {
    vector<size_t> outliers;
    for (const Cluster& cluster : clusters) {
        if (cluster.memberIndices.size() < 3) {
            continue;
        }
        vector<double> distances;
        distances.reserve(cluster.memberIndices.size());
        for (size_t pointIndex : cluster.memberIndices) {
            distances.push_back(distance(data[pointIndex], cluster.centroid));
        }
        const double mean = accumulate(distances.begin(), distances.end(), 0.0) /
                            static_cast<double>(distances.size());
        double variance = 0.0;
        for (double memberDistance : distances) {
            variance += (memberDistance - mean) * (memberDistance - mean);
        }
        const double standardDeviation = sqrt(variance / static_cast<double>(distances.size()));
        if (standardDeviation == 0.0) {
            continue;
        }
        for (size_t index = 0; index < cluster.memberIndices.size(); ++index) {
            if ((distances[index] - mean) / standardDeviation >= zScoreThreshold) {
                outliers.push_back(cluster.memberIndices[index]);
            }
        }
    }
    sort(outliers.begin(), outliers.end());
    return outliers;
}
