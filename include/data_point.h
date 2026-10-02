#pragma once

#include <cstddef>
#include <ostream>

struct DataPoint {
    double x{};
    double y{};

    DataPoint() = default;
    DataPoint(double x_value, double y_value) : x(x_value), y(y_value) {}

    double norm2() const;
    DataPoint& operator+=(const DataPoint& other);
};

double squaredDistance(const DataPoint& first, const DataPoint& second);
double euclideanDistance(const DataPoint& first, const DataPoint& second);
DataPoint operator+(const DataPoint& first, const DataPoint& second);
DataPoint operator-(const DataPoint& first, const DataPoint& second);
DataPoint operator*(const DataPoint& point, double scalar);
DataPoint operator*(double scalar, const DataPoint& point);
DataPoint operator/(const DataPoint& point, std::size_t divisor);
bool operator==(const DataPoint& first, const DataPoint& second);
std::ostream& operator<<(std::ostream& output, const DataPoint& point);
