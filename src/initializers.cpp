#include "initializers.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace {
void validateInput(const vector<DataPoint>& data, size_t clusterCount) {
    if (clusterCount == 0 || clusterCount > data.size()) {
        throw invalid_argument("cluster count must be between 1 and the number of points");
    }
}
}  // namespace

vector<DataPoint> RandomInitializer::initialize(const vector<DataPoint>& data,
                                                 size_t clusterCount,
                                                 mt19937& generator) const {
    validateInput(data, clusterCount);
    vector<size_t> indices(data.size());
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), generator);

    vector<DataPoint> centroids;
    centroids.reserve(clusterCount);
    for (size_t index = 0; index < clusterCount; ++index) {
        centroids.push_back(data[indices[index]]);
    }
    return centroids;
}

vector<DataPoint> KMeansPlusPlusInitializer::initialize(const vector<DataPoint>& data,
                                                         size_t clusterCount,
                                                         mt19937& generator) const {
    validateInput(data, clusterCount);
    uniform_int_distribution<size_t> firstPoint(0, data.size() - 1);
    const size_t initialIndex = firstPoint(generator);
    vector<DataPoint> centroids{data[initialIndex]};
    vector<double> weights(data.size());
    vector<bool> chosen(data.size(), false);
    chosen[initialIndex] = true;

    while (centroids.size() < clusterCount) {
        for (size_t point = 0; point < data.size(); ++point) {
            double nearest = squaredDistance(data[point], centroids.front());
            for (size_t centroid = 1; centroid < centroids.size(); ++centroid) {
                nearest = min(nearest, squaredDistance(data[point], centroids[centroid]));
            }
            weights[point] = chosen[point] ? 0.0 : nearest;
        }

        const double totalWeight = accumulate(weights.begin(), weights.end(), 0.0);
        size_t selected = 0;
        if (totalWeight == 0.0) {
            vector<size_t> remaining;
            for (size_t point = 0; point < data.size(); ++point) {
                if (!chosen[point]) remaining.push_back(point);
            }
            uniform_int_distribution<size_t> chooseRemaining(0, remaining.size() - 1);
            selected = remaining[chooseRemaining(generator)];
        } else {
            discrete_distribution<size_t> choosePoint(weights.begin(), weights.end());
            selected = choosePoint(generator);
        }
        centroids.push_back(data[selected]);
        chosen[selected] = true;
    }
    return centroids;
}
