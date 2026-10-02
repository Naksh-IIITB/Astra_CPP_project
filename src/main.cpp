#include "data_loader.h"
#include "interactive_cli.h"
#include "pipeline.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
struct CommandOptions {
    PipelineOptions pipeline;
    CsvOptions csv;
    std::string input;
    bool demo = false;
};

void printUsage() {
    std::cout << "MiniCluster - explainable 2D K-Means\n\n"
              << "Usage: minicluster [--input file.csv | --demo] [options]\n"
              << "       minicluster --interactive\n\n"
              << "  --k N              number of clusters (default: 3)\n"
              << "  --init NAME        kmeans++ or random (default: kmeans++)\n"
              << "  --metric NAME      euclidean, manhattan, or chebyshev\n"
              << "  --seed N           deterministic random seed (default: 42)\n"
              << "  --max-iterations N convergence limit (default: 200)\n"
              << "  --x-col N          zero-based x column (default: 0)\n"
              << "  --y-col N          zero-based y column (default: 1)\n"
              << "  --delimiter C      comma, semicolon, or tab character\n"
              << "  --no-header        treat first content row as data\n"
              << "  --strict           fail instead of skipping invalid rows\n"
              << "  --export file.csv  write point-to-cluster assignments\n"
              << "  --graph file       write matching .svg and .png plots\n";
}

std::size_t parseSize(const std::string& value, const std::string& option, bool allowZero = false) {
    try {
        std::size_t consumed = 0;
        const unsigned long parsed = std::stoul(value, &consumed);
        if (consumed != value.size() || (!allowZero && parsed == 0)) throw std::invalid_argument("value");
        return static_cast<std::size_t>(parsed);
    } catch (const std::exception&) {
        throw std::invalid_argument(option + " needs a " + (allowZero ? "non-negative" : "positive") + " integer");
    }
}

CommandOptions parseOptions(int argc, char* argv[]) {
    CommandOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        const auto value = [&]() -> std::string {
            if (++index >= argc) throw std::invalid_argument(argument + " needs a value");
            return argv[index];
        };
        if (argument == "--help" || argument == "-h") { printUsage(); std::exit(EXIT_SUCCESS); }
        else if (argument == "--demo") options.demo = true;
        else if (argument == "--k") options.pipeline.clusterCount = parseSize(value(), argument);
        else if (argument == "--max-iterations") options.pipeline.maxIterations = parseSize(value(), argument);
        else if (argument == "--seed") options.pipeline.seed = static_cast<unsigned int>(parseSize(value(), argument, true));
        else if (argument == "--init") options.pipeline.initializerName = value();
        else if (argument == "--metric") options.pipeline.metricName = value();
        else if (argument == "--input") options.input = value();
        else if (argument == "--export") options.pipeline.exportFile = value();
        else if (argument == "--graph") options.pipeline.graphFile = value();
        else if (argument == "--x-col") options.csv.xColumn = parseSize(value(), argument, true);
        else if (argument == "--y-col") options.csv.yColumn = parseSize(value(), argument, true);
        else if (argument == "--delimiter") {
            const std::string delimiter = value();
            if (delimiter.size() != 1 || (delimiter[0] != ',' && delimiter[0] != ';' && delimiter[0] != '\t')) {
                throw std::invalid_argument("--delimiter must be ',', ';', or a tab character");
            }
            options.csv.delimiter = delimiter[0];
        } else if (argument == "--no-header") options.csv.hasHeader = false;
        else if (argument == "--strict") options.csv.skipBadRows = false;
        else if (argument == "--interactive") throw std::invalid_argument("--interactive cannot be combined with other options");
        else throw std::invalid_argument("unknown option: " + argument);
    }
    if ((options.demo && !options.input.empty()) || (!options.demo && options.input.empty())) {
        throw std::invalid_argument("choose exactly one of --demo or --input");
    }
    return options;
}
}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--interactive")) {
            InteractiveCli().run();
            return EXIT_SUCCESS;
        }
        const CommandOptions options = parseOptions(argc, argv);
        std::unique_ptr<DataLoader> loader = options.demo
            ? std::unique_ptr<DataLoader>(std::make_unique<DemoLoader>(options.pipeline.seed))
            : makeLoader(options.input, options.csv);
        Pipeline(std::move(loader), options.pipeline).run(std::cout);
        return EXIT_SUCCESS;
    } catch (const LoadError& error) {
        std::cerr << "Load error: " << error.what() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
    }
    return EXIT_FAILURE;
}
