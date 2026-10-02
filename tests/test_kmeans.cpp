#include "analytics.h"
#include "distance_metric.h"
#include "initializers.h"
#include "kmeans.h"
#include "data_loader.h"
#include "interactive_cli.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

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
    const SilhouetteMetric silhouetteMetric;
    assert(silhouetteMetric.compute(data, model.clusters()) > 0.8);

    const std::string csvPath = "/private/tmp/minicluster_loader.csv";
    const std::string semicolonPath = "/private/tmp/minicluster_loader_semicolon.csv";
    const std::string crlfPath = "/private/tmp/minicluster_loader_crlf.csv";
    const std::string badPath = "/private/tmp/minicluster_loader_bad.csv";
    { std::ofstream csv(csvPath); csv << "x,y\n1,2\n3,4\n"; }
    { std::ofstream csv(semicolonPath); csv << "x;y\n1;2\n"; }
    { std::ofstream csv(crlfPath, std::ios::binary); csv << "x,y\r\n1,2\r\n"; }
    { std::ofstream csv(badPath); csv << "x,y\n1,2\nbad,row\n"; }
    assert(CsvLoader(csvPath).load().size() == 2);
    assert(CsvLoader(semicolonPath).load().size() == 1);
    assert(CsvLoader(crlfPath).load().size() == 1);
    assert(CsvLoader(badPath).load().size() == 1);
    bool missingFileFailed = false;
    try { CsvLoader("/private/tmp/no-such-minicluster-file.csv").load(); }
    catch (const LoadError&) { missingFileFailed = true; }
    assert(missingFileFailed);

    std::istringstream interactiveInput(
        "1\n"
        "data/mall_customers_large.csv\n\n\n\n"
        "2\n6\n3\nkmeans++\n5\n42\n6\n0\n");
    std::ostringstream interactiveOutput;
    InteractiveCli(interactiveInput, interactiveOutput).run();
    assert(interactiveOutput.str().find("inertia: 211729.084") != std::string::npos);

    std::istringstream invalidInput("6\ninvalid\n0\n");
    std::ostringstream invalidOutput;
    InteractiveCli(invalidInput, invalidOutput).run();
    assert(invalidOutput.str().find("Load data before using this action.") != std::string::npos);
    assert(invalidOutput.str().find("Choose a menu number") != std::string::npos);

    std::istringstream eofInput("1\n");
    std::ostringstream eofOutput;
    InteractiveCli(eofInput, eofOutput).run();
    std::cout << "All MiniCluster tests passed.\n";
}
