#include "kmeans.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

KMeans::KMeans(std::size_t clusterCount,
               std::unique_ptr<CentroidInitializer> initializer,
               std::size_t maxIterations,
               double tolerance,
               unsigned int seed,
               std::unique_ptr<DistanceMetric> metric)
    : clusterCount_(clusterCount), initializer_(std::move(initializer)), metric_(std::move(metric)),
      maxIterations_(maxIterations), tolerance_(tolerance), generator_(seed) {
    if (clusterCount == 0 || maxIterations == 0 || tolerance < 0.0 || !initializer_ || !metric_) {
        throw std::invalid_argument("invalid K-Means configuration");
    }
}

void KMeans::fit(const std::vector<DataPoint>& data) {
    if (data.empty() || clusterCount_ > data.size()) {
        throw std::invalid_argument("data must contain at least as many points as clusters");
    }

    clusters_.clear();
    for (const DataPoint& centroid : initializer_->initialize(data, clusterCount_, generator_, *metric_)) {
        clusters_.push_back({centroid, {}});
    }

    converged_ = false;
    iterations_ = 0;
    for (; iterations_ < maxIterations_; ++iterations_) {
        assignPoints(data);
        if (updateCentroids(data)) {
            converged_ = true;
            ++iterations_;
            break;
        }
    }

    assignPoints(data);
    inertia_ = 0.0;
    for (const Cluster& cluster : clusters_) {
        for (size_t pointIndex : cluster.memberIndices) {
            inertia_ += squaredDistance(data[pointIndex], cluster.centroid);
        }
    }
}

void KMeans::assignPoints(const std::vector<DataPoint>& data) {
    for (Cluster& cluster : clusters_) {
        cluster.memberIndices.clear();
    }
    for (std::size_t pointIndex = 0; pointIndex < data.size(); ++pointIndex) {
        std::size_t nearestCluster = 0;
        double nearestDistance = (*metric_)(data[pointIndex], clusters_.front().centroid);
        for (std::size_t clusterIndex = 1; clusterIndex < clusters_.size(); ++clusterIndex) {
            const double candidate = (*metric_)(data[pointIndex], clusters_[clusterIndex].centroid);
            if (candidate < nearestDistance) {
                nearestDistance = candidate;
                nearestCluster = clusterIndex;
            }
        }
        clusters_[nearestCluster].memberIndices.push_back(pointIndex);
    }
}

bool KMeans::updateCentroids(const std::vector<DataPoint>& data) {
    double greatestShift = 0.0;
    for (std::size_t index = 0; index < clusters_.size(); ++index) {
        Cluster& cluster = clusters_[index];
        DataPoint updated{};
        if (cluster.memberIndices.empty()) {
            // Recover an empty cluster by relocating it to the most poorly represented point.
            std::size_t farthestPoint = 0;
            double farthestDistance = -1.0;
            for (std::size_t point = 0; point < data.size(); ++point) {
                double nearestDistance = std::numeric_limits<double>::infinity();
                for (const Cluster& candidate : clusters_) {
                    nearestDistance = std::min(nearestDistance,
                                          (*metric_)(data[point], candidate.centroid));
                }
                if (nearestDistance > farthestDistance) {
                    farthestDistance = nearestDistance;
                    farthestPoint = point;
                }
            }
            updated = data[farthestPoint];
        } else {
            for (size_t pointIndex : cluster.memberIndices) {
                updated += data[pointIndex];
            }
            updated = updated / cluster.memberIndices.size();
        }
        greatestShift = std::max(greatestShift, (*metric_)(cluster.centroid, updated));
        cluster.centroid = updated;
    }
    return greatestShift <= tolerance_;
}

const std::vector<Cluster>& KMeans::clusters() const noexcept { return clusters_; }
std::size_t KMeans::iterations() const noexcept { return iterations_; }
bool KMeans::converged() const noexcept { return converged_; }
double KMeans::inertia() const noexcept { return inertia_; }
const DistanceMetric& KMeans::metric() const noexcept { return *metric_; }

void KMeans::printResults(std::ostream& output) const {
    output << std::fixed << std::setprecision(3);
    for (std::size_t index = 0; index < clusters_.size(); ++index) {
        const Cluster& cluster = clusters_[index];
        output << "Cluster " << index + 1 << ": centroid " << cluster.centroid
               << ", members: " << cluster.memberIndices.size() << '\n';
    }
}
