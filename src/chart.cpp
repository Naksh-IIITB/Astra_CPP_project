#include "chart.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace {
constexpr double kCanvasWidth = 1000.0;
constexpr double kCanvasHeight = 680.0;
constexpr double kLeft = 95.0;
constexpr double kRight = 50.0;
constexpr double kTop = 55.0;
constexpr double kBottom = 90.0;
constexpr array<const char*, 8> kColors{
    "#2563eb", "#dc2626", "#16a34a", "#9333ea",
    "#ea580c", "#0891b2", "#be123c", "#4f46e5"};

struct Bounds {
    double minX;
    double maxX;
    double minY;
    double maxY;
};

Bounds boundsFor(const vector<DataPoint>& data) {
    Bounds bounds{data.front().x, data.front().x, data.front().y, data.front().y};
    for (const DataPoint& point : data) {
        bounds.minX = min(bounds.minX, point.x);
        bounds.maxX = max(bounds.maxX, point.x);
        bounds.minY = min(bounds.minY, point.y);
        bounds.maxY = max(bounds.maxY, point.y);
    }
    const double xPadding = max(1.0, (bounds.maxX - bounds.minX) * 0.08);
    const double yPadding = max(1.0, (bounds.maxY - bounds.minY) * 0.08);
    return {bounds.minX - xPadding, bounds.maxX + xPadding,
            bounds.minY - yPadding, bounds.maxY + yPadding};
}

double screenX(double value, const Bounds& bounds) {
    return kLeft + (value - bounds.minX) / (bounds.maxX - bounds.minX) *
                   (kCanvasWidth - kLeft - kRight);
}

double screenY(double value, const Bounds& bounds) {
    return kCanvasHeight - kBottom - (value - bounds.minY) / (bounds.maxY - bounds.minY) *
                                      (kCanvasHeight - kTop - kBottom);
}
}  // namespace

void writeClusterSvg(const string& filename,
                     const vector<DataPoint>& data,
                     const vector<Cluster>& clusters) {
    if (data.empty()) {
        throw invalid_argument("cannot chart an empty dataset");
    }
    ofstream output(filename);
    if (!output) {
        throw runtime_error("could not write graph file: " + filename);
    }

    const Bounds bounds = boundsFor(data);
    const double plotWidth = kCanvasWidth - kLeft - kRight;
    const double plotHeight = kCanvasHeight - kTop - kBottom;
    output << fixed << setprecision(2);
    output << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1000\" height=\"680\" "
           << "viewBox=\"0 0 1000 680\">\n"
           << "<rect width=\"1000\" height=\"680\" fill=\"#ffffff\"/>\n"
           << "<text x=\"95\" y=\"32\" font-family=\"Arial, sans-serif\" font-size=\"22\" "
           << "font-weight=\"bold\" fill=\"#111827\">MiniCluster: K-Means Result</text>\n"
           << "<rect x=\"" << kLeft << "\" y=\"" << kTop << "\" width=\"" << plotWidth
           << "\" height=\"" << plotHeight << "\" fill=\"#f8fafc\" stroke=\"#cbd5e1\"/>\n";

    for (int tick = 0; tick <= 5; ++tick) {
        const double x = kLeft + plotWidth * tick / 5.0;
        const double y = kTop + plotHeight * tick / 5.0;
        const double xValue = bounds.minX + (bounds.maxX - bounds.minX) * tick / 5.0;
        const double yValue = bounds.maxY - (bounds.maxY - bounds.minY) * tick / 5.0;
        output << "<line x1=\"" << x << "\" y1=\"" << kTop << "\" x2=\"" << x
               << "\" y2=\"" << kCanvasHeight - kBottom << "\" stroke=\"#e2e8f0\"/>\n"
               << "<line x1=\"" << kLeft << "\" y1=\"" << y << "\" x2=\""
               << kCanvasWidth - kRight << "\" y2=\"" << y << "\" stroke=\"#e2e8f0\"/>\n"
               << "<text x=\"" << x << "\" y=\"" << kCanvasHeight - 58
               << "\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\"12\" fill=\"#475569\">"
               << xValue << "</text>\n"
               << "<text x=\"75\" y=\"" << y + 4
               << "\" text-anchor=\"end\" font-family=\"Arial\" font-size=\"12\" fill=\"#475569\">"
               << yValue << "</text>\n";
    }

    for (size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        const char* color = kColors[clusterIndex % kColors.size()];
        for (size_t pointIndex : clusters[clusterIndex].memberIndices) {
            const DataPoint& point = data[pointIndex];
            output << "<circle cx=\"" << screenX(point.x, bounds) << "\" cy=\""
                   << screenY(point.y, bounds) << "\" r=\"4\" fill=\"" << color
                   << "\" fill-opacity=\"0.68\"/>\n";
        }
    }

    for (size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        const Cluster& cluster = clusters[clusterIndex];
        const char* color = kColors[clusterIndex % kColors.size()];
        const double x = screenX(cluster.centroid.x, bounds);
        const double y = screenY(cluster.centroid.y, bounds);
        output << "<path d=\"M " << x - 8 << ' ' << y - 8 << " L " << x + 8 << ' ' << y + 8
               << " M " << x + 8 << ' ' << y - 8 << " L " << x - 8 << ' ' << y + 8
               << "\" stroke=\"" << color << "\" stroke-width=\"4\"/>\n"
               << "<text x=\"" << x + 12 << "\" y=\"" << y - 10
               << "\" font-family=\"Arial\" font-size=\"13\" font-weight=\"bold\" fill=\""
               << color << "\">C" << clusterIndex + 1 << " (" << cluster.memberIndices.size()
               << ")</text>\n";
    }

    output << "<text x=\"" << kCanvasWidth / 2.0 << "\" y=\"655\" text-anchor=\"middle\" "
           << "font-family=\"Arial\" font-size=\"14\" fill=\"#334155\">Annual income (k$)</text>\n"
           << "<text x=\"22\" y=\"" << kCanvasHeight / 2.0 << "\" text-anchor=\"middle\" "
           << "transform=\"rotate(-90 22 " << kCanvasHeight / 2.0 << ")\" font-family=\"Arial\" "
           << "font-size=\"14\" fill=\"#334155\">Spending score</text>\n"
           << "<text x=\"760\" y=\"32\" font-family=\"Arial\" font-size=\"12\" fill=\"#475569\">"
           << "Dots: customers   X: centroids</text>\n</svg>\n";
}
