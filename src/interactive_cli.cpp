#include "interactive_cli.h"

#include "exporter.h"
#include "initializers.h"

#include <iomanip>
#include <limits>
#include <sstream>

namespace {
std::string readLine(Session& session, const std::string& prompt, const std::string& fallback = "") {
    session.output << prompt;
    if (!fallback.empty()) session.output << " [" << fallback << ']';
    session.output << ": ";
    std::string value;
    if (!std::getline(session.input, value)) { session.running = false; return ""; }
    return value.empty() ? fallback : value;
}

std::size_t readInt(Session& session, const std::string& prompt, std::size_t minimum,
                    std::size_t maximum, std::size_t fallback) {
    while (session.running) {
        const std::string value = readLine(session, prompt, std::to_string(fallback));
        if (!session.running) return fallback;
        try {
            std::size_t consumed = 0;
            const unsigned long parsed = std::stoul(value, &consumed);
            if (consumed == value.size() && parsed >= minimum && parsed <= maximum) return parsed;
        } catch (const std::exception&) {}
        session.output << "Please enter a number from " << minimum << " to " << maximum << ".\n";
    }
    return fallback;
}

std::string readChoice(Session& session, const std::string& prompt,
                       const std::vector<std::string>& options, const std::string& fallback) {
    while (session.running) {
        const std::string value = readLine(session, prompt, fallback);
        if (!session.running) return fallback;
        for (const std::string& option : options) if (value == option) return value;
        session.output << "Choose one of: ";
        for (std::size_t index = 0; index < options.size(); ++index) {
            session.output << options[index] << (index + 1 == options.size() ? "\n" : ", ");
        }
    }
    return fallback;
}

bool requireData(Session& session) {
    if (!session.data.empty()) return true;
    session.output << "Load data before using this action.\n";
    return false;
}

bool requireResult(Session& session) {
    if (session.lastModel) return true;
    session.output << "Run clustering before using this action.\n";
    return false;
}

void runModel(Session& session, const std::string& initializerName) {
    auto model = std::make_unique<KMeans>(session.k, makeInitializer(initializerName),
                                          session.maxIterations, 1e-4, session.seed,
                                          makeMetric(session.metricName));
    model->fit(session.data);
    const InertiaMetric inertia;
    const SilhouetteMetric silhouette(makeMetric(session.metricName));
    session.lastInertia = inertia.compute(session.data, model->clusters());
    session.lastSilhouette = silhouette.compute(session.data, model->clusters());
    session.lastModel = std::move(model);
}

class LoadDataAction final : public MenuAction {
public:
    std::string label() const override { return "Load data"; }
    void run(Session& session) override {
        const std::string path = readLine(session, "File path");
        if (!session.running || path.empty()) return;
        CsvOptions options;
        const std::string x = readLine(session, "X column", "0");
        const std::string y = readLine(session, "Y column", "1");
        const std::string delimiter = readLine(session, "Delimiter (auto, comma, semicolon, tab)", "auto");
        try {
            options.xColumn = static_cast<std::size_t>(std::stoul(x));
            options.yColumn = static_cast<std::size_t>(std::stoul(y));
            if (delimiter == "comma") options.delimiter = ',';
            else if (delimiter == "semicolon") options.delimiter = ';';
            else if (delimiter == "tab") options.delimiter = '\t';
            else if (delimiter != "auto") throw std::invalid_argument("delimiter must be auto, comma, semicolon, or tab");
            std::unique_ptr<DataLoader> loader = makeLoader(path, options);
            session.data = loader->load();
            session.dataDescription = loader->describe();
            session.lastModel.reset();
            session.output << "Loaded " << session.data.size() << " rows from " << session.dataDescription << ".\n";
        } catch (const std::exception& error) { session.output << "Load failed: " << error.what() << '\n'; }
    }
};

class ChooseKAction final : public MenuAction {
public:
    std::string label() const override { return "Set k"; }
    void run(Session& session) override {
        if (!requireData(session)) return;
        session.k = readInt(session, "Number of clusters", 1, session.data.size(), session.k);
    }
};

class ChooseInitializerAction final : public MenuAction {
public:
    std::string label() const override { return "Choose initializer"; }
    void run(Session& session) override {
        session.initializerName = readChoice(session, "Initializer", {"random", "kmeans++"}, session.initializerName);
    }
};

class ChooseMetricAction final : public MenuAction {
public:
    std::string label() const override { return "Choose distance metric"; }
    void run(Session& session) override {
        session.metricName = readChoice(session, "Metric", {"euclidean", "manhattan", "chebyshev"}, session.metricName);
    }
};

class SetSeedAction final : public MenuAction {
public:
    std::string label() const override { return "Set seed"; }
    void run(Session& session) override {
        const std::string value = readLine(session, "Seed (Enter keeps current)", std::to_string(session.seed));
        if (!session.running) return;
        try { session.seed = static_cast<unsigned int>(std::stoul(value)); }
        catch (const std::exception&) { session.output << "Seed must be a non-negative integer.\n"; }
    }
};

class RunClusteringAction final : public MenuAction {
public:
    std::string label() const override { return "Run clustering"; }
    void run(Session& session) override {
        if (!requireData(session)) return;
        if (session.k > session.data.size()) { session.output << "k cannot exceed the row count.\n"; return; }
        runModel(session, session.initializerName);
        const ClusterDiagnostics diagnostics(session.lastModel->metric());
        session.output << std::fixed << std::setprecision(3)
                       << "Iterations: " << session.lastModel->iterations()
                       << " | converged: " << (session.lastModel->converged() ? "yes" : "no")
                       << " | inertia: " << session.lastInertia
                       << " | silhouette: " << session.lastSilhouette << '\n';
        session.output << "Cluster | Size | Average radius | Max radius\n";
        for (const ClusterSummary& summary : diagnostics.summarize(session.data, session.lastModel->clusters())) {
            session.output << summary.index + 1 << "       | " << summary.size << "    | "
                           << summary.averageDistance << "          | " << summary.maximumDistance << '\n';
        }
    }
};

class ShowOutliersAction final : public MenuAction {
public:
    std::string label() const override { return "Show outlier candidates"; }
    void run(Session& session) override {
        if (!requireResult(session)) return;
        const ClusterDiagnostics diagnostics(session.lastModel->metric());
        const auto outliers = diagnostics.findOutliers(session.data, session.lastModel->clusters());
        session.output << "Outlier candidates: ";
        if (outliers.empty()) session.output << "none";
        for (std::size_t index : outliers) session.output << '#' << index << ' ';
        session.output << '\n';
    }
};

class ExportResultsAction final : public MenuAction {
public:
    std::string label() const override { return "Export results"; }
    void run(Session& session) override {
        if (!requireResult(session)) return;
        const std::string kind = readChoice(session, "Format", {"csv", "svg", "png"}, "csv");
        const std::string file = readLine(session, "Output filename");
        if (!session.running || file.empty()) return;
        std::unique_ptr<Exporter> exporter;
        if (kind == "csv") exporter = std::make_unique<CsvExporter>();
        else if (kind == "svg") exporter = std::make_unique<SvgExporter>();
        else exporter = std::make_unique<PngExporter>();
        exporter->write(file, session.data, session.lastModel->clusters());
        session.output << "Wrote " << file << ".\n";
    }
};

class CompareInitializersAction final : public MenuAction {
public:
    std::string label() const override { return "Compare initializers"; }
    void run(Session& session) override {
        if (!requireData(session)) return;
        session.output << "Initializer | Inertia | Silhouette\n";
        for (const std::string& name : {std::string("random"), std::string("kmeans++")}) {
            runModel(session, name);
            session.output << name << " | " << std::fixed << std::setprecision(3)
                           << session.lastInertia << " | " << session.lastSilhouette << '\n';
        }
    }
};

class QuitAction final : public MenuAction {
public:
    std::string label() const override { return "Quit"; }
    void run(Session& session) override { session.running = false; }
};
}  // namespace

InteractiveCli::InteractiveCli(std::istream& input, std::ostream& output) : session_(input, output) {
    actions_.emplace_back(std::make_unique<LoadDataAction>());
    actions_.emplace_back(std::make_unique<ChooseKAction>());
    actions_.emplace_back(std::make_unique<ChooseInitializerAction>());
    actions_.emplace_back(std::make_unique<ChooseMetricAction>());
    actions_.emplace_back(std::make_unique<SetSeedAction>());
    actions_.emplace_back(std::make_unique<RunClusteringAction>());
    actions_.emplace_back(std::make_unique<ShowOutliersAction>());
    actions_.emplace_back(std::make_unique<ExportResultsAction>());
    actions_.emplace_back(std::make_unique<CompareInitializersAction>());
    actions_.emplace_back(std::make_unique<QuitAction>());
}

void InteractiveCli::run() {
    while (session_.running) {
        session_.output << "\nMiniCluster interactive mode\n";
        for (std::size_t index = 0; index < actions_.size() - 1; ++index) {
            session_.output << index + 1 << ". " << actions_[index]->label() << '\n';
        }
        session_.output << "0. " << actions_.back()->label() << '\n';
        const std::string choice = readLine(session_, "Choice");
        if (!session_.running) break;
        try {
            const int selected = std::stoi(choice);
            if (selected == 0) actions_.back()->run(session_);
            else if (selected >= 1 && static_cast<std::size_t>(selected) < actions_.size()) {
                actions_[static_cast<std::size_t>(selected - 1)]->run(session_);
            } else session_.output << "Choose a menu number from 0 to 9.\n";
        } catch (const std::exception&) { session_.output << "Choose a menu number from 0 to 9.\n"; }
    }
}
