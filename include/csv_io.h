#pragma once

#include "data_point.h"

#include <string>
#include <vector>

std::vector<DataPoint> readPointsFromCsv(const std::string& filename);
