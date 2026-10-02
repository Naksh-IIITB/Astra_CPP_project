#include "data_point.h"

#include <cmath>

double squaredDistance(const DataPoint& first, const DataPoint& second) {
    return (first - second).norm2();
}

double euclideanDistance(const DataPoint& first, const DataPoint& second) {
    return std::sqrt(squaredDistance(first, second));
}

DataPoint operator+(const DataPoint& first, const DataPoint& second) {
    return {first.x + second.x, first.y + second.y};
}

DataPoint operator-(const DataPoint& first, const DataPoint& second) {
    return {first.x - second.x, first.y - second.y};
}

DataPoint operator*(const DataPoint& point, double scalar) {
    return {point.x * scalar, point.y * scalar};
}

DataPoint operator*(double scalar, const DataPoint& point) {
    return point * scalar;
}

DataPoint operator/(const DataPoint& point, std::size_t divisor) {
    return {point.x / static_cast<double>(divisor), point.y / static_cast<double>(divisor)};
}

double DataPoint::norm2() const {
    return x * x + y * y;
}

DataPoint& DataPoint::operator+=(const DataPoint& other) {
    x += other.x;
    y += other.y;
    return *this;
}

bool operator==(const DataPoint& first, const DataPoint& second) {
    return first.x == second.x && first.y == second.y;
}

std::ostream& operator<<(std::ostream& output, const DataPoint& point) {
    return output << '(' << point.x << ", " << point.y << ')';
}
