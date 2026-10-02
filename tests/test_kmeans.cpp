#include "analytics.h"
#include "distance_metric.h"
#include "initializers.h"
#include "kmeans.h"

#include <cassert>
#include <iostream>

using namespace std;

int main() {
    const DataPoint origin{0, 0};
    const DataPoint point{3, 4};
    const EuclideanMetric euclidean;
    const ManhattanMetric manhattan;
    const ChebyshevMetric chebyshev;
    assert(euclidean(origin, point) == 5.0);
    assert(manhattan(origin, point) == 7.0);
    assert(chebyshev(origin, point) == 4.0);

    const vector<DataPoint> data{{0, 0}, {0, 1}, {1, 0}, {10, 10}, {10, 11}, {11, 10}};
    KMeans model(2, std::make_unique<KMeansPlusPlusInitializer>(), 100, 1e-8, 17);
    model.fit(data);

    assert(model.converged());
    assert(model.clusters().size() == 2);
    assert(model.clusters()[0].memberIndices.size() + model.clusters()[1].memberIndices.size() == data.size());
    assert(model.inertia() < 5.0);
    assert(silhouetteScore(data, model.clusters(), model.metric()) > 0.8);
    std::cout << "All MiniCluster tests passed.\n";
}
