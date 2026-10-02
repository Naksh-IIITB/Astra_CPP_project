#pragma once

#include "kmeans.h"

#include <string>
#include <vector>

class Exporter {
public:
    virtual ~Exporter() = default;
    virtual void write(const std::string& file, const std::vector<DataPoint>& data,
                       const std::vector<Cluster>& clusters) const = 0;
    virtual std::string name() const = 0;
};

class SvgExporter final : public Exporter {
public:
    void write(const std::string& file, const std::vector<DataPoint>& data,
               const std::vector<Cluster>& clusters) const override;
    std::string name() const override;
};

class PngExporter final : public Exporter {
public:
    void write(const std::string& file, const std::vector<DataPoint>& data,
               const std::vector<Cluster>& clusters) const override;
    std::string name() const override;
};

class CsvExporter final : public Exporter {
public:
    void write(const std::string& file, const std::vector<DataPoint>& data,
               const std::vector<Cluster>& clusters) const override;
    std::string name() const override;
};
