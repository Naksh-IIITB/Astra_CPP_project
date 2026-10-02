#include "data_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

namespace {
char detectDelimiter(const std::string& line) {
    if (line.find('\t') != std::string::npos) return '\t';
    if (line.find(';') != std::string::npos) return ';';
    return ',';
}

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, delimiter)) fields.push_back(field);
    return fields;
}

std::string lowerExtension(const std::string& path) {
    const std::size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string extension = path.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });
    return extension;
}
}  // namespace

CsvLoader::CsvLoader(std::string path, CsvOptions options)
    : path_(std::move(path)), options_(options) {}

std::vector<DataPoint> CsvLoader::load() const {
    std::ifstream input(path_);
    if (!input) throw LoadError("could not open input file: " + path_);

    std::vector<DataPoint> data;
    std::string line;
    std::size_t lineNumber = 0;
    bool firstContentRow = true;
    char delimiter = options_.delimiter;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (delimiter == '\0') delimiter = detectDelimiter(line);
        if (firstContentRow && options_.hasHeader) {
            firstContentRow = false;
            continue;
        }
        firstContentRow = false;
        DataPoint point;
        if (parseRow(line, delimiter, point)) {
            data.push_back(point);
        } else if (options_.skipBadRows) {
            std::cerr << "Warning: skipping invalid row " << path_ << ':' << lineNumber << '\n';
        } else {
            throw LoadError("invalid numeric row at " + path_ + ':' + std::to_string(lineNumber));
        }
    }
    if (data.empty()) throw LoadError("no valid numeric rows found in: " + path_);
    return data;
}

std::string CsvLoader::describe() const { return "CSV data from " + path_; }

bool CsvLoader::parseRow(const std::string& line, char delimiter, DataPoint& point) const {
    const std::vector<std::string> fields = split(line, delimiter);
    const std::size_t needed = std::max(options_.xColumn, options_.yColumn);
    if (fields.size() <= needed) return false;
    try {
        std::size_t parsedX = 0;
        std::size_t parsedY = 0;
        point.x = std::stod(fields[options_.xColumn], &parsedX);
        point.y = std::stod(fields[options_.yColumn], &parsedY);
        return parsedX == fields[options_.xColumn].size() && parsedY == fields[options_.yColumn].size();
    } catch (const std::exception&) {
        return false;
    }
}

TsvLoader::TsvLoader(std::string path, CsvOptions options) : CsvLoader(std::move(path), [&]() {
    options.delimiter = '\t';
    return options;
}()) {}

std::vector<DataPoint> DemoLoader::load() const {
    std::mt19937 generator(seed_);
    const std::vector<DataPoint> centers{{20, 80}, {52, 45}, {82, 18}};
    std::normal_distribution<double> spread(0.0, 6.5);
    std::vector<DataPoint> data;
    for (const DataPoint& center : centers) {
        for (int index = 0; index < 35; ++index) {
            data.push_back({center.x + spread(generator), center.y + spread(generator)});
        }
    }
    data.push_back({5, 8});
    return data;
}

std::string DemoLoader::describe() const { return "built-in synthetic customer demo"; }

std::unique_ptr<DataLoader> makeLoader(const std::string& path, CsvOptions options) {
    const std::string extension = lowerExtension(path);
    if (extension == ".tsv" || extension == ".tab") return std::make_unique<TsvLoader>(path, options);
    return std::make_unique<CsvLoader>(path, options);
}
