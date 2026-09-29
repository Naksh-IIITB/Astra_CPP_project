#include "kmeans.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

KMeans::KMeans(size_t clusterCount,
               const CentroidInitializer& initializer,
               size_t maxIterations,
               double tolerance,
               unsigned int seed)
    : clusterCount_(clusterCount), initializer_(initializer), maxIterations_(maxIterations),
      tolerance_(tolerance), generator_(seed) {
    if (clusterCount == 0 || maxIterations == 0 || tolerance < 0.0) {
        throw invalid_argument("invalid K-Means configuration");
    }
}

void KMeans::fit(const vector<DataPoint>& data) {
    if (data.empty() || clusterCount_ > data.size()) {
        throw invalid_argument("data must contain at least as many points as clusters");
    }

    clusters_.clear();
    for (const DataPoint& centroid : initializer_.initialize(data, clusterCount_, generator_)) {
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

void KMeans::assignPoints(const vector<DataPoint>& data) {
    for (Cluster& cluster : clusters_) {
        cluster.memberIndices.clear();
    }
    for (size_t pointIndex = 0; pointIndex < data.size(); ++pointIndex) {
        size_t nearestCluster = 0;
        double nearestDistance = squaredDistance(data[pointIndex], clusters_.front().centroid);
        for (size_t clusterIndex = 1; clusterIndex < clusters_.size(); ++clusterIndex) {
            const double candidate = squaredDistance(data[pointIndex], clusters_[clusterIndex].centroid);
            if (candidate < nearestDistance) {
                nearestDistance = candidate;
                nearestCluster = clusterIndex;
            }
        }
        clusters_[nearestCluster].memberIndices.push_back(pointIndex);
    }
}

bool KMeans::updateCentroids(const vector<DataPoint>& data) {
    double greatestShift = 0.0;
    for (size_t index = 0; index < clusters_.size(); ++index) {
        Cluster& cluster = clusters_[index];
        DataPoint updated{};
        if (cluster.memberIndices.empty()) {
            // Recover an empty cluster by relocating it to the most poorly represented point.
            size_t farthestPoint = 0;
            double farthestDistance = -1.0;
            for (size_t point = 0; point < data.size(); ++point) {
                double nearestDistance = numeric_limits<double>::infinity();
                for (const Cluster& candidate : clusters_) {
                    nearestDistance = min(nearestDistance,
                                          squaredDistance(data[point], candidate.centroid));
                }
                if (nearestDistance > farthestDistance) {
                    farthestDistance = nearestDistance;
                    farthestPoint = point;
                }
            }
            updated = data[farthestPoint];
        } else {
            for (size_t pointIndex : cluster.memberIndices) {
                updated = updated + data[pointIndex];
            }
            updated = updated / cluster.memberIndices.size();
        }
        greatestShift = max(greatestShift, distance(cluster.centroid, updated));
        cluster.centroid = updated;
    }
    return greatestShift <= tolerance_;
}

const vector<Cluster>& KMeans::clusters() const noexcept { return clusters_; }
size_t KMeans::iterations() const noexcept { return iterations_; }
bool KMeans::converged() const noexcept { return converged_; }
double KMeans::inertia() const noexcept { return inertia_; }

void KMeans::printResults() const {
    cout << fixed << setprecision(3);
    for (size_t index = 0; index < clusters_.size(); ++index) {
        const Cluster& cluster = clusters_[index];
        cout << "Cluster " << index + 1 << ": centroid (" << cluster.centroid.x << ", "
             << cluster.centroid.y << "), members: " << cluster.memberIndices.size() << '\n';
    }
}
