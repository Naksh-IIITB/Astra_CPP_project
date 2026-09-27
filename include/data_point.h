#pragma once

#include <cstddef>

using namespace std;

struct DataPoint {
    double x{};
    double y{};

    DataPoint() = default;
    DataPoint(double x_value, double y_value) : x(x_value), y(y_value) {}
};

double squaredDistance(const DataPoint& first, const DataPoint& second);
double distance(const DataPoint& first, const DataPoint& second);
DataPoint operator+(const DataPoint& first, const DataPoint& second);
DataPoint operator/(const DataPoint& point, size_t divisor);
