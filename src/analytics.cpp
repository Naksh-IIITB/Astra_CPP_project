#include "analytics.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {
double averageDistanceToCluster(const DataPoint& point, std::size_t ownIndex,
                                const std::vector<DataPoint>& data, const Cluster& cluster,
                                const DistanceMetric& metric) {
    double total = 0.0;
    std::size_t count = 0;
    for (std::size_t member : cluster.memberIndices) {
        if (member != ownIndex) { total += metric(point, data[member]); ++count; }
    }
    return count == 0 ? 0.0 : total / static_cast<double>(count);
}
}  // namespace

double InertiaMetric::compute(const std::vector<DataPoint>& data,
                              const std::vector<Cluster>& clusters) const {
    double inertia = 0.0;
    for (const Cluster& cluster : clusters) {
        for (std::size_t pointIndex : cluster.memberIndices) {
            inertia += squaredDistance(data[pointIndex], cluster.centroid);
        }
    }
    return inertia;
}

std::string InertiaMetric::name() const { return "inertia"; }

SilhouetteMetric::SilhouetteMetric(std::unique_ptr<DistanceMetric> metric) : metric_(std::move(metric)) {
    if (!metric_) throw std::invalid_argument("silhouette metric requires a distance metric");
}

double SilhouetteMetric::compute(const std::vector<DataPoint>& data,
                                 const std::vector<Cluster>& clusters) const {
    if (data.size() < 2 || clusters.size() < 2) return 0.0;
    double total = 0.0;
    std::size_t evaluated = 0;
    for (std::size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        for (std::size_t pointIndex : clusters[clusterIndex].memberIndices) {
            const double intra = averageDistanceToCluster(data[pointIndex], pointIndex, data,
                                                          clusters[clusterIndex], *metric_);
            double nearestOther = std::numeric_limits<double>::infinity();
            for (std::size_t other = 0; other < clusters.size(); ++other) {
                if (other != clusterIndex && !clusters[other].memberIndices.empty()) {
                    nearestOther = std::min(nearestOther, averageDistanceToCluster(
                        data[pointIndex], pointIndex, data, clusters[other], *metric_));
                }
            }
            const double denominator = std::max(intra, nearestOther);
            total += denominator == 0.0 ? 0.0 : (nearestOther - intra) / denominator;
            ++evaluated;
        }
    }
    return evaluated == 0 ? 0.0 : total / static_cast<double>(evaluated);
}

std::string SilhouetteMetric::name() const { return "silhouette (" + metric_->name() + ")"; }

std::vector<ClusterSummary> ClusterDiagnostics::summarize(
    const std::vector<DataPoint>& data, const std::vector<Cluster>& clusters) const {
    std::vector<ClusterSummary> summaries;
    summaries.reserve(clusters.size());
    for (std::size_t index = 0; index < clusters.size(); ++index) {
        const Cluster& cluster = clusters[index];
        double totalDistance = 0.0;
        double maximumDistance = 0.0;
        for (std::size_t pointIndex : cluster.memberIndices) {
            const double memberDistance = metric_(data[pointIndex], cluster.centroid);
            totalDistance += memberDistance;
            maximumDistance = std::max(maximumDistance, memberDistance);
        }
        const double average = cluster.memberIndices.empty() ? 0.0
            : totalDistance / static_cast<double>(cluster.memberIndices.size());
        summaries.push_back({index, cluster.memberIndices.size(), average, maximumDistance});
    }
    return summaries;
}

std::vector<std::size_t> ClusterDiagnostics::findOutliers(
    const std::vector<DataPoint>& data, const std::vector<Cluster>& clusters,
    double zScoreThreshold) const {
    std::vector<std::size_t> outliers;
    for (const Cluster& cluster : clusters) {
        if (cluster.memberIndices.size() < 3) continue;
        std::vector<double> distances;
        distances.reserve(cluster.memberIndices.size());
        for (std::size_t pointIndex : cluster.memberIndices) {
            distances.push_back(metric_(data[pointIndex], cluster.centroid));
        }
        const double mean = std::accumulate(distances.begin(), distances.end(), 0.0) /
                            static_cast<double>(distances.size());
        double variance = 0.0;
        for (double memberDistance : distances) variance += (memberDistance - mean) * (memberDistance - mean);
        const double deviation = std::sqrt(variance / static_cast<double>(distances.size()));
        if (deviation == 0.0) continue;
        for (std::size_t index = 0; index < cluster.memberIndices.size(); ++index) {
            if ((distances[index] - mean) / deviation >= zScoreThreshold) {
                outliers.push_back(cluster.memberIndices[index]);
            }
        }
    }
    std::sort(outliers.begin(), outliers.end());
    return outliers;
}
