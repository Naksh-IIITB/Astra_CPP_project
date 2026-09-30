#include "chart.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

using namespace std;

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

struct Rgb { uint8_t red; uint8_t green; uint8_t blue; };
constexpr array<Rgb, 8> kPngColors{{
    {37, 99, 235}, {220, 38, 38}, {22, 163, 74}, {147, 51, 234},
    {234, 88, 12}, {8, 145, 178}, {190, 18, 60}, {79, 70, 229}}};

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

class Raster {
public:
    Raster(int width, int height, Rgb background)
        : width_(width), height_(height), pixels_(static_cast<size_t>(width * height), background) {}

    void pixel(int x, int y, Rgb color) {
        if (x >= 0 && x < width_ && y >= 0 && y < height_) {
            pixels_[static_cast<size_t>(y * width_ + x)] = color;
        }
    }

    void line(int x0, int y0, int x1, int y1, Rgb color) {
        const int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        const int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int error = dx + dy;
        while (true) {
            pixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            const int twiceError = 2 * error;
            if (twiceError >= dy) { error += dy; x0 += sx; }
            if (twiceError <= dx) { error += dx; y0 += sy; }
        }
    }

    void circle(int centerX, int centerY, int radius, Rgb color) {
        for (int y = -radius; y <= radius; ++y) {
            for (int x = -radius; x <= radius; ++x) {
                if (x * x + y * y <= radius * radius) pixel(centerX + x, centerY + y, color);
            }
        }
    }

    const vector<Rgb>& pixels() const { return pixels_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    int width_;
    int height_;
    vector<Rgb> pixels_;
};

uint32_t crc32(const vector<uint8_t>& bytes) {
    uint32_t crc = 0xffffffffU;
    for (uint8_t byte : bytes) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xedb88320U & -(crc & 1U));
    }
    return ~crc;
}

uint32_t adler32(const vector<uint8_t>& bytes) {
    uint32_t first = 1, second = 0;
    for (uint8_t byte : bytes) {
        first = (first + byte) % 65521U;
        second = (second + first) % 65521U;
    }
    return (second << 16) | first;
}

void appendUint32(vector<uint8_t>& bytes, uint32_t value) {
    bytes.push_back(static_cast<uint8_t>(value >> 24));
    bytes.push_back(static_cast<uint8_t>(value >> 16));
    bytes.push_back(static_cast<uint8_t>(value >> 8));
    bytes.push_back(static_cast<uint8_t>(value));
}

void writeChunk(ofstream& output, const char type[4], const vector<uint8_t>& data) {
    vector<uint8_t> checksumData{static_cast<uint8_t>(type[0]), static_cast<uint8_t>(type[1]),
                                 static_cast<uint8_t>(type[2]), static_cast<uint8_t>(type[3])};
    checksumData.insert(checksumData.end(), data.begin(), data.end());
    vector<uint8_t> header;
    appendUint32(header, static_cast<uint32_t>(data.size()));
    output.write(reinterpret_cast<const char*>(header.data()), static_cast<streamsize>(header.size()));
    output.write(type, 4);
    if (!data.empty()) output.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
    vector<uint8_t> checksum;
    appendUint32(checksum, crc32(checksumData));
    output.write(reinterpret_cast<const char*>(checksum.data()), static_cast<streamsize>(checksum.size()));
}

void writePng(const string& filename, const Raster& image) {
    ofstream output(filename, ios::binary);
    if (!output) throw runtime_error("could not write PNG graph file: " + filename);
    const uint8_t signature[] = {137, 80, 78, 71, 13, 10, 26, 10};
    output.write(reinterpret_cast<const char*>(signature), sizeof(signature));
    vector<uint8_t> header;
    appendUint32(header, static_cast<uint32_t>(image.width()));
    appendUint32(header, static_cast<uint32_t>(image.height()));
    header.insert(header.end(), {8, 2, 0, 0, 0});
    writeChunk(output, "IHDR", header);

    vector<uint8_t> raw;
    raw.reserve(static_cast<size_t>(image.height() * (image.width() * 3 + 1)));
    for (int y = 0; y < image.height(); ++y) {
        raw.push_back(0);
        for (int x = 0; x < image.width(); ++x) {
            const Rgb color = image.pixels()[static_cast<size_t>(y * image.width() + x)];
            raw.insert(raw.end(), {color.red, color.green, color.blue});
        }
    }
    vector<uint8_t> compressed{0x78, 0x01};
    for (size_t offset = 0; offset < raw.size();) {
        const size_t length = min<size_t>(65535, raw.size() - offset);
        compressed.push_back(offset + length == raw.size() ? 1 : 0);
        compressed.push_back(static_cast<uint8_t>(length));
        compressed.push_back(static_cast<uint8_t>(length >> 8));
        const uint16_t complement = static_cast<uint16_t>(~length);
        compressed.push_back(static_cast<uint8_t>(complement));
        compressed.push_back(static_cast<uint8_t>(complement >> 8));
        compressed.insert(compressed.end(), raw.begin() + static_cast<ptrdiff_t>(offset),
                          raw.begin() + static_cast<ptrdiff_t>(offset + length));
        offset += length;
    }
    appendUint32(compressed, adler32(raw));
    writeChunk(output, "IDAT", compressed);
    writeChunk(output, "IEND", {});
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

void writeClusterPng(const string& filename,
                     const vector<DataPoint>& data,
                     const vector<Cluster>& clusters) {
    if (data.empty()) throw invalid_argument("cannot chart an empty dataset");
    const Bounds bounds = boundsFor(data);
    Raster image(static_cast<int>(kCanvasWidth), static_cast<int>(kCanvasHeight), {255, 255, 255});
    const Rgb grid{226, 232, 240};
    const Rgb border{148, 163, 184};
    const int left = static_cast<int>(kLeft), right = static_cast<int>(kCanvasWidth - kRight);
    const int top = static_cast<int>(kTop), bottom = static_cast<int>(kCanvasHeight - kBottom);
    for (int tick = 0; tick <= 5; ++tick) {
        const int x = left + (right - left) * tick / 5;
        const int y = top + (bottom - top) * tick / 5;
        image.line(x, top, x, bottom, grid);
        image.line(left, y, right, y, grid);
    }
    image.line(left, top, right, top, border);
    image.line(right, top, right, bottom, border);
    image.line(right, bottom, left, bottom, border);
    image.line(left, bottom, left, top, border);
    for (size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        const Rgb color = kPngColors[clusterIndex % kPngColors.size()];
        for (size_t pointIndex : clusters[clusterIndex].memberIndices) {
            image.circle(static_cast<int>(screenX(data[pointIndex].x, bounds)),
                         static_cast<int>(screenY(data[pointIndex].y, bounds)), 4, color);
        }
    }
    for (size_t clusterIndex = 0; clusterIndex < clusters.size(); ++clusterIndex) {
        const Rgb color = kPngColors[clusterIndex % kPngColors.size()];
        const int x = static_cast<int>(screenX(clusters[clusterIndex].centroid.x, bounds));
        const int y = static_cast<int>(screenY(clusters[clusterIndex].centroid.y, bounds));
        image.line(x - 9, y - 9, x + 9, y + 9, color);
        image.line(x + 9, y - 9, x - 9, y + 9, color);
    }
    writePng(filename, image);
}
