#include "pipeline.h"

#include "initializers.h"

#include <iomanip>
#include <stdexcept>

namespace {
std::string graphBase(std::string filename) {
    for (const std::string& extension : {std::string(".svg"), std::string(".png")}) {
        if (filename.size() >= extension.size() &&
            filename.compare(filename.size() - extension.size(), extension.size(), extension) == 0) {
            filename.resize(filename.size() - extension.size());
        }
    }
    return filename;
}
}  // namespace

Pipeline::Pipeline(std::unique_ptr<DataLoader> loader, PipelineOptions options)
    : loader_(std::move(loader)), options_(std::move(options)) {
    if (!loader_) throw std::invalid_argument("pipeline requires a data loader");
    metrics_.emplace_back(std::make_unique<InertiaMetric>());
    metrics_.emplace_back(std::make_unique<SilhouetteMetric>(makeMetric(options_.metricName)));
    exporters_.emplace_back(std::make_unique<CsvExporter>());
    exporters_.emplace_back(std::make_unique<SvgExporter>());
    exporters_.emplace_back(std::make_unique<PngExporter>());
}

void Pipeline::run(std::ostream& output) {
    const std::vector<DataPoint> data = loader_->load();
    model_ = std::make_unique<KMeans>(options_.clusterCount, makeInitializer(options_.initializerName),
                                      options_.maxIterations, options_.tolerance, options_.seed,
                                      makeMetric(options_.metricName));
    model_->fit(data);

    const ClusterDiagnostics diagnostics(model_->metric());
    const auto summaries = diagnostics.summarize(data, model_->clusters());
    const auto outliers = diagnostics.findOutliers(data, model_->clusters());
    const double inertia = metrics_[0]->compute(data, model_->clusters());
    const double silhouette = metrics_[1]->compute(data, model_->clusters());

    output << std::fixed << std::setprecision(3);
    output << "MiniCluster report\n==================\n"
           << "Points: " << data.size() << " | K: " << options_.clusterCount
           << " | initializer: " << options_.initializerName;
    if (options_.metricName != "euclidean") output << " | metric: " << options_.metricName;
    output << " | seed: " << options_.seed << "\n"
           << "Iterations: " << model_->iterations() << " | converged: "
           << (model_->converged() ? "yes" : "no") << " | inertia: " << inertia
           << " | silhouette: " << silhouette << "\n\n";
    model_->printResults();
    output << "\nCompactness diagnostics\n";
    for (const ClusterSummary& summary : summaries) {
        output << "  C" << summary.index + 1 << ": avg radius " << summary.averageDistance
               << ", max radius " << summary.maximumDistance << '\n';
    }
    output << "Outlier candidates (z >= 2.5): ";
    if (outliers.empty()) output << "none\n";
    else { for (std::size_t index : outliers) output << '#' << index << ' '; output << '\n'; }

    for (const auto& exporter : exporters_) {
        if (exporter->name() == "csv" && !options_.exportFile.empty()) {
            exporter->write(options_.exportFile, data, model_->clusters());
            output << "\nAssignments written to " << options_.exportFile << '\n';
        }
        if ((exporter->name() == "svg" || exporter->name() == "png") && !options_.graphFile.empty()) {
            exporter->write(graphBase(options_.graphFile) + "." + exporter->name(), data, model_->clusters());
        }
    }
    if (!options_.graphFile.empty()) {
        const std::string base = graphBase(options_.graphFile);
        output << "Cluster graphs written to " << base << ".svg and " << base << ".png\n";
    }
}
