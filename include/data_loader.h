#pragma once

#include "data_point.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class LoadError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class DataLoader {
public:
    virtual ~DataLoader() = default;
    virtual std::vector<DataPoint> load() const = 0;
    virtual std::string describe() const = 0;
};

struct CsvOptions {
    char delimiter = '\0';
    std::size_t xColumn = 0;
    std::size_t yColumn = 1;
    bool hasHeader = true;
    bool skipBadRows = true;
};

class CsvLoader : public DataLoader {
public:
    CsvLoader(std::string path, CsvOptions options = {});
    std::vector<DataPoint> load() const override;
    std::string describe() const override;

protected:
    virtual bool parseRow(const std::string& line, char delimiter, DataPoint& point) const;

private:
    std::string path_;
    CsvOptions options_;
};

class TsvLoader final : public CsvLoader {
public:
    explicit TsvLoader(std::string path, CsvOptions options = {});
};

class DemoLoader final : public DataLoader {
public:
    explicit DemoLoader(unsigned int seed = 42) : seed_(seed) {}
    std::vector<DataPoint> load() const override;
    std::string describe() const override;

private:
    unsigned int seed_;
};

std::unique_ptr<DataLoader> makeLoader(const std::string& path, CsvOptions options = {});
