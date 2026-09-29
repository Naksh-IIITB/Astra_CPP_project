#include "analytics.h"
#include "initializers.h"
#include "kmeans.h"

#include <cassert>
#include <iostream>

int main() {
    const vector<DataPoint> data{{0, 0}, {0, 1}, {1, 0}, {10, 10}, {10, 11}, {11, 10}};
    KMeansPlusPlusInitializer initializer;
    KMeans model(2, initializer, 100, 1e-8, 17);
    model.fit(data);

    assert(model.converged());
    assert(model.clusters().size() == 2);
    assert(model.clusters()[0].memberIndices.size() + model.clusters()[1].memberIndices.size() == data.size());
    assert(model.inertia() < 5.0);
    assert(silhouetteScore(data, model.clusters()) > 0.8);
    std::cout << "All MiniCluster tests passed.\n";
}
