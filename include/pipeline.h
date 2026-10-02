#pragma once

#include "analytics.h"
#include "data_loader.h"
#include "exporter.h"

#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct PipelineOptions {
    std::size_t clusterCount = 3;
    std::size_t maxIterations = 200;
    double tolerance = 1e-4;
    unsigned int seed = 42;
    std::string initializerName = "kmeans++";
    std::string metricName = "euclidean";
    std::string exportFile;
    std::string graphFile;
};

class Pipeline {
public:
    Pipeline(std::unique_ptr<DataLoader> loader, PipelineOptions options);
    void run(std::ostream& output);

private:
    std::unique_ptr<DataLoader> loader_;
    PipelineOptions options_;
    std::unique_ptr<KMeans> model_;
    std::vector<std::unique_ptr<ClusterMetric>> metrics_;
    std::vector<std::unique_ptr<Exporter>> exporters_;
};
