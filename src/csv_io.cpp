#include "csv_io.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
bool parsePoint(const string& line, DataPoint& point) {
    string normalized = line;
    for (char& character : normalized) {
        if (character == ';' || character == '\t') {
            character = ',';
        }
    }
    string first;
    string second;
    stringstream row(normalized);
    if (!getline(row, first, ',') || !getline(row, second, ',')) {
        return false;
    }
    try {
        size_t parsedFirst = 0;
        size_t parsedSecond = 0;
        point.x = stod(first, &parsedFirst);
        point.y = stod(second, &parsedSecond);
        return parsedFirst == first.size() && parsedSecond == second.size();
    } catch (const exception&) {
        return false;
    }
}
}  // namespace

vector<DataPoint> readPointsFromCsv(const string& filename) {
    ifstream input(filename);
    if (!input) {
        throw runtime_error("could not open input file: " + filename);
    }
    vector<DataPoint> data;
    string line;
    while (getline(input, line)) {
        DataPoint point;
        if (!line.empty() && parsePoint(line, point)) {
            data.push_back(point);
        }
    }
    if (data.empty()) {
        throw runtime_error("no valid numeric x,y rows found in: " + filename);
    }
    return data;
}

void writeAssignmentsToCsv(const string& filename,
                           const vector<DataPoint>& data,
                           const vector<Cluster>& clusters) {
    ofstream output(filename);
    if (!output) {
        throw runtime_error("could not write output file: " + filename);
    }
    output << "point_id,x,y,cluster\n";
    output << fixed << setprecision(6);
    for (size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        for (size_t pointIndex : clusters[clusterIndex].memberIndices) {
            output << pointIndex << ',' << data[pointIndex].x << ',' << data[pointIndex].y << ','
                   << clusterIndex + 1 << '\n';
        }
    }
}
