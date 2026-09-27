#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

struct DataPoint {
    std::vector<double> values;

    double distanceSquared(const DataPoint& other) const {
        if (values.size() != other.values.size()) {
            throw std::invalid_argument("Points must have the same dimensionality");
        }

        double distance = 0.0;
        for (std::size_t i = 0; i < values.size(); ++i) {
            const double difference = values[i] - other.values[i];
            distance += difference * difference;
        }
        return distance;
    }
};

struct Cluster {
    DataPoint centroid;
    std::vector<std::size_t> memberIndices;
};

class Initialiser {
public:
    virtual ~Initialiser() = default;
    virtual std::vector<DataPoint> initialise(
        const std::vector<DataPoint>& data, int k, std::mt19937& generator) const = 0;
};

class RandomInitialiser final : public Initialiser {
public:
    std::vector<DataPoint> initialise(
        const std::vector<DataPoint>& data, int k, std::mt19937& generator) const override {
        std::vector<std::size_t> indices(data.size());
        for (std::size_t i = 0; i < indices.size(); ++i) {
            indices[i] = i;
        }
        std::shuffle(indices.begin(), indices.end(), generator);

        std::vector<DataPoint> centroids;
        for (int i = 0; i < k; ++i) {
            centroids.push_back(data[indices[static_cast<std::size_t>(i)]]);
        }
        return centroids;
    }
};

class KMeansPlusPlusInitialiser final : public Initialiser {
public:
    std::vector<DataPoint> initialise(
        const std::vector<DataPoint>& data, int k, std::mt19937& generator) const override {
        std::uniform_int_distribution<std::size_t> firstPick(0, data.size() - 1);
        std::vector<DataPoint> centroids{data[firstPick(generator)]};
        std::vector<double> distances(data.size(), std::numeric_limits<double>::max());

        while (static_cast<int>(centroids.size()) < k) {
            double totalDistance = 0.0;
            for (std::size_t i = 0; i < data.size(); ++i) {
                distances[i] = std::min(distances[i], data[i].distanceSquared(centroids.back()));
                totalDistance += distances[i];
            }

            std::size_t selected = 0;
            if (totalDistance > 0.0) {
                std::uniform_real_distribution<double> pick(0.0, totalDistance);
                const double target = pick(generator);
                double cumulative = 0.0;
                for (std::size_t i = 0; i < distances.size(); ++i) {
                    cumulative += distances[i];
                    if (cumulative >= target) {
                        selected = i;
                        break;
                    }
                }
            } else {
                std::uniform_int_distribution<std::size_t> pick(0, data.size() - 1);
                selected = pick(generator);
            }
            centroids.push_back(data[selected]);
        }
        return centroids;
    }
};

class KMeans {
public:
    KMeans(int k, int maxIterations, const Initialiser& initialiser, unsigned seed = 42)
        : k_(k), maxIterations_(maxIterations), initialiser_(initialiser), generator_(seed) {}

    void fit(const std::vector<DataPoint>& data) {
        validate(data);
        std::vector<DataPoint> centroids = initialiser_.initialise(data, k_, generator_);
        clusters_.assign(static_cast<std::size_t>(k_), Cluster{});

        for (int iteration = 0; iteration < maxIterations_; ++iteration) {
            for (std::size_t i = 0; i < clusters_.size(); ++i) {
                clusters_[i].centroid = centroids[i];
                clusters_[i].memberIndices.clear();
            }

            for (std::size_t pointIndex = 0; pointIndex < data.size(); ++pointIndex) {
                std::size_t nearest = 0;
                double bestDistance = data[pointIndex].distanceSquared(centroids[0]);
                for (std::size_t clusterIndex = 1; clusterIndex < centroids.size(); ++clusterIndex) {
                    const double distance = data[pointIndex].distanceSquared(centroids[clusterIndex]);
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        nearest = clusterIndex;
                    }
                }
                clusters_[nearest].memberIndices.push_back(pointIndex);
            }

            std::vector<DataPoint> updated = centroids;
            for (std::size_t clusterIndex = 0; clusterIndex < clusters_.size(); ++clusterIndex) {
                const auto& members = clusters_[clusterIndex].memberIndices;
                if (members.empty()) {
                    std::uniform_int_distribution<std::size_t> pick(0, data.size() - 1);
                    updated[clusterIndex] = data[pick(generator_)];
                    continue;
                }

                updated[clusterIndex].values.assign(data.front().values.size(), 0.0);
                for (std::size_t pointIndex : members) {
                    for (std::size_t dimension = 0; dimension < data[pointIndex].values.size(); ++dimension) {
                        updated[clusterIndex].values[dimension] += data[pointIndex].values[dimension];
                    }
                }
                for (double& value : updated[clusterIndex].values) {
                    value /= static_cast<double>(members.size());
                }
            }

            bool converged = true;
            for (std::size_t i = 0; i < centroids.size(); ++i) {
                if (centroids[i].distanceSquared(updated[i]) > 1e-10) {
                    converged = false;
                    break;
                }
            }
            centroids = std::move(updated);
            if (converged) {
                break;
            }
        }

        for (std::size_t i = 0; i < clusters_.size(); ++i) {
            clusters_[i].centroid = centroids[i];
        }
    }

    void printResults(const std::vector<DataPoint>& data) const {
        std::cout << std::fixed << std::setprecision(2);
        for (std::size_t clusterIndex = 0; clusterIndex < clusters_.size(); ++clusterIndex) {
            const auto& cluster = clusters_[clusterIndex];
            std::cout << "Cluster " << clusterIndex + 1 << "\n  Centroid: (";
            for (std::size_t i = 0; i < cluster.centroid.values.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << cluster.centroid.values[i];
            }
            std::cout << ")\n  Size: " << cluster.memberIndices.size() << "\n  Members: ";
            for (std::size_t i = 0; i < cluster.memberIndices.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << "(";
                const auto& point = data[cluster.memberIndices[i]];
                for (std::size_t dimension = 0; dimension < point.values.size(); ++dimension) {
                    if (dimension > 0) std::cout << ", ";
                    std::cout << point.values[dimension];
                }
                std::cout << ")";
            }
            std::cout << "\n\n";
        }
    }

private:
    void validate(const std::vector<DataPoint>& data) const {
        if (data.empty() || k_ <= 0 || k_ > static_cast<int>(data.size()) || maxIterations_ <= 0) {
            throw std::invalid_argument("Require non-empty data, 1 <= k <= data size, and positive iterations");
        }
        const std::size_t dimensions = data.front().values.size();
        if (dimensions == 0) {
            throw std::invalid_argument("Points must have at least one dimension");
        }
        for (const auto& point : data) {
            if (point.values.size() != dimensions) {
                throw std::invalid_argument("All points must have the same dimensionality");
            }
        }
    }

    int k_;
    int maxIterations_;
    const Initialiser& initialiser_;
    std::mt19937 generator_;
    std::vector<Cluster> clusters_;
};

int main() {
    const std::vector<DataPoint> data = {
        {{1.0, 2.0}}, {{1.5, 1.8}}, {{2.0, 2.2}}, {{2.5, 1.5}},
        {{8.0, 8.5}}, {{8.5, 9.0}}, {{9.0, 8.0}}, {{9.5, 9.2}},
        {{19.0, 20.0}}, {{20.0, 19.5}}, {{21.0, 20.5}}
    };

    try {
        KMeansPlusPlusInitialiser initialiser;
        KMeans model(3, 100, initialiser);
        model.fit(data);
        model.printResults(data);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
