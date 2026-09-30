#include "data_point.h"

#include <cmath>

using namespace std;

double squaredDistance(const DataPoint& first, const DataPoint& second) {
    const double dx = first.x - second.x;
    const double dy = first.y - second.y;
    return dx * dx + dy * dy;
}

double distance(const DataPoint& first, const DataPoint& second) {
    return std::sqrt(squaredDistance(first, second));
}

DataPoint operator+(const DataPoint& first, const DataPoint& second) {
    return {first.x + second.x, first.y + second.y};
}

DataPoint operator/(const DataPoint& point, size_t divisor) {
    return {point.x / static_cast<double>(divisor), point.y / static_cast<double>(divisor)};
}
