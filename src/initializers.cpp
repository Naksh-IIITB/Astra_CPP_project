#include "initializers.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace {
void validateInput(const std::vector<DataPoint>& data, std::size_t clusterCount) {
    if (clusterCount == 0 || clusterCount > data.size()) {
        throw std::invalid_argument("cluster count must be between 1 and the number of points");
    }
}
}  // namespace

std::vector<DataPoint> RandomInitializer::initialize(const std::vector<DataPoint>& data,
                                                      std::size_t clusterCount,
                                                      std::mt19937& generator) const {
    validateInput(data, clusterCount);
    std::vector<std::size_t> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), generator);

    std::vector<DataPoint> centroids;
    centroids.reserve(clusterCount);
    for (std::size_t index = 0; index < clusterCount; ++index) {
        centroids.push_back(data[indices[index]]);
    }
    return centroids;
}

std::vector<DataPoint> KMeansPlusPlusInitializer::initialize(const std::vector<DataPoint>& data,
                                                              std::size_t clusterCount,
                                                              std::mt19937& generator) const {
    validateInput(data, clusterCount);
    std::uniform_int_distribution<std::size_t> firstPoint(0, data.size() - 1);
    const std::size_t initialIndex = firstPoint(generator);
    std::vector<DataPoint> centroids{data[initialIndex]};
    std::vector<double> weights(data.size());
    std::vector<bool> chosen(data.size(), false);
    chosen[initialIndex] = true;

    while (centroids.size() < clusterCount) {
        for (std::size_t point = 0; point < data.size(); ++point) {
            double nearest = squaredDistance(data[point], centroids.front());
            for (std::size_t centroid = 1; centroid < centroids.size(); ++centroid) {
                nearest = std::min(nearest, squaredDistance(data[point], centroids[centroid]));
            }
            weights[point] = chosen[point] ? 0.0 : nearest;
        }

        const double totalWeight = std::accumulate(weights.begin(), weights.end(), 0.0);
        std::size_t selected = 0;
        if (totalWeight == 0.0) {
            std::vector<std::size_t> remaining;
            for (std::size_t point = 0; point < data.size(); ++point) {
                if (!chosen[point]) remaining.push_back(point);
            }
            std::uniform_int_distribution<std::size_t> chooseRemaining(0, remaining.size() - 1);
            selected = remaining[chooseRemaining(generator)];
        } else {
            std::discrete_distribution<std::size_t> choosePoint(weights.begin(), weights.end());
            selected = choosePoint(generator);
        }
        centroids.push_back(data[selected]);
        chosen[selected] = true;
    }
    return centroids;
}
