#include "exporter.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>

void CsvExporter::write(const std::string& file, const std::vector<DataPoint>& data,
                        const std::vector<Cluster>& clusters) const {
    std::ofstream output(file);
    if (!output) throw std::runtime_error("could not write output file: " + file);
    output << "point_id,x,y,cluster\n" << std::fixed << std::setprecision(6);
    for (std::size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        for (std::size_t pointIndex : clusters[clusterIndex].memberIndices) {
            output << pointIndex << ',' << data[pointIndex].x << ',' << data[pointIndex].y << ','
                   << clusterIndex + 1 << '\n';
        }
    }
}

std::string CsvExporter::name() const { return "csv"; }
