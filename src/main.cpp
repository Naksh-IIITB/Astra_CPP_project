#include "analytics.h"
#include "chart.h"
#include "csv_io.h"
#include "initializers.h"
#include "kmeans.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

namespace {
struct Options {
    size_t clusters = 3;
    size_t maxIterations = 200;
    unsigned int seed = 42;
    string initializer = "kmeans++";
    string input;
    string exportFile;
    string graphFile;
    bool demo = false;
    bool map = false;
};

void printUsage() {
    cout << "MiniCluster - explainable 2D K-Means\n\n"
         << "Usage: minicluster [--input file.csv | --demo] [options]\n"
         << "  --k N              number of clusters (default: 3)\n"
         << "  --init NAME        kmeans++ or random (default: kmeans++)\n"
         << "  --seed N           deterministic random seed (default: 42)\n"
         << "  --max-iterations N convergence limit (default: 200)\n"
         << "  --export file.csv  write point-to-cluster assignments\n"
         << "  --graph file.svg   write a color-coded cluster scatter plot\n"
         << "  --map              print an ASCII cluster map\n";
}

size_t positiveSize(const string& value, const string& option) {
    try {
        const unsigned long parsed = stoul(value);
        if (parsed == 0) throw invalid_argument("zero");
        return static_cast<size_t>(parsed);
    } catch (const exception&) {
        throw invalid_argument(option + " needs a positive integer");
    }
}

Options parseOptions(int argc, char* argv[]) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const string argument = argv[index];
        const auto value = [&]() -> string {
            if (++index >= argc) throw invalid_argument(argument + " needs a value");
            return argv[index];
        };
        if (argument == "--help" || argument == "-h") {
            printUsage();
            exit(EXIT_SUCCESS);
        } else if (argument == "--demo") options.demo = true;
        else if (argument == "--map") options.map = true;
        else if (argument == "--k") options.clusters = positiveSize(value(), argument);
        else if (argument == "--max-iterations") options.maxIterations = positiveSize(value(), argument);
        else if (argument == "--seed") options.seed = static_cast<unsigned int>(positiveSize(value(), argument));
        else if (argument == "--init") options.initializer = value();
        else if (argument == "--input") options.input = value();
        else if (argument == "--export") options.exportFile = value();
        else if (argument == "--graph") options.graphFile = value();
        else throw invalid_argument("unknown option: " + argument);
    }
    if ((options.demo && !options.input.empty()) || (!options.demo && options.input.empty())) {
        throw invalid_argument("choose exactly one of --demo or --input");
    }
    if (options.initializer != "kmeans++" && options.initializer != "random") {
        throw invalid_argument("--init must be kmeans++ or random");
    }
    return options;
}

vector<DataPoint> demoData(unsigned int seed) {
    mt19937 generator(seed);
    const vector<DataPoint> centers{{20, 80}, {52, 45}, {82, 18}};
    normal_distribution<double> spread(0.0, 6.5);
    vector<DataPoint> data;
    for (const DataPoint& center : centers) {
        for (int index = 0; index < 35; ++index) data.push_back({center.x + spread(generator), center.y + spread(generator)});
    }
    data.push_back({5, 8});  // A deliberately unusual customer for the outlier report.
    return data;
}

void printMap(const vector<DataPoint>& data, const vector<Cluster>& clusters) {
    constexpr int width = 56;
    constexpr int height = 18;
    double minX = data.front().x, maxX = data.front().x, minY = data.front().y, maxY = data.front().y;
    for (const auto& point : data) {
        minX = min(minX, point.x); maxX = max(maxX, point.x);
        minY = min(minY, point.y); maxY = max(maxY, point.y);
    }
    vector<string> canvas(height, string(width, ' '));
    for (size_t cluster = 0; cluster < clusters.size(); ++cluster) {
        const char marker = static_cast<char>('1' + (cluster % 9));
        for (size_t point : clusters[cluster].memberIndices) {
            const int x = static_cast<int>((data[point].x - minX) / max(0.001, maxX - minX) * (width - 1));
            const int y = static_cast<int>((data[point].y - minY) / max(0.001, maxY - minY) * (height - 1));
            canvas[height - 1 - y][x] = marker;
        }
    }
    cout << "\nCluster map (x: " << minX << " to " << maxX << ", y: " << minY << " to " << maxY << ")\n";
    for (const string& row : canvas) cout << '|' << row << "|\n";
}
}  // namespace

int main(int argc, char* argv[]) {
    try {
        const Options options = parseOptions(argc, argv);
        const vector<DataPoint> data = options.demo ? demoData(options.seed) : readPointsFromCsv(options.input);
        unique_ptr<CentroidInitializer> initializer = options.initializer == "random"
            ? unique_ptr<CentroidInitializer>(new RandomInitializer())
            : unique_ptr<CentroidInitializer>(new KMeansPlusPlusInitializer());

        KMeans model(options.clusters, *initializer, options.maxIterations, 1e-4, options.seed);
        model.fit(data);
        const auto summaries = summarizeClusters(data, model.clusters());
        const auto outliers = findOutliers(data, model.clusters());

        cout << fixed << setprecision(3);
        cout << "MiniCluster report\n"
             << "==================\n"
             << "Points: " << data.size() << " | K: " << options.clusters
             << " | initializer: " << options.initializer << " | seed: " << options.seed << "\n"
             << "Iterations: " << model.iterations() << " | converged: " << (model.converged() ? "yes" : "no")
             << " | inertia: " << model.inertia() << " | silhouette: "
             << silhouetteScore(data, model.clusters()) << "\n\n";
        model.printResults();
        cout << "\nCompactness diagnostics\n";
        for (const auto& summary : summaries) {
            cout << "  C" << summary.index + 1 << ": avg radius " << summary.averageDistance
                 << ", max radius " << summary.maximumDistance << '\n';
        }
        cout << "Outlier candidates (z >= 2.5): ";
        if (outliers.empty()) cout << "none\n";
        else {
            for (size_t index : outliers) cout << '#' << index << ' ';
            cout << '\n';
        }
        if (options.map) printMap(data, model.clusters());
        if (!options.exportFile.empty()) {
            writeAssignmentsToCsv(options.exportFile, data, model.clusters());
            cout << "\nAssignments written to " << options.exportFile << '\n';
        }
        if (!options.graphFile.empty()) {
            writeClusterSvg(options.graphFile, data, model.clusters());
            cout << "Cluster graph written to " << options.graphFile << '\n';
        }
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << "\n\n";
        printUsage();
        return EXIT_FAILURE;
    }
}
