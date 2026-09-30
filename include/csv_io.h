#pragma once

#include "kmeans.h"

#include <string>
#include <vector>

using namespace std;

vector<DataPoint> readPointsFromCsv(const string& filename);
void writeAssignmentsToCsv(const string& filename,
                           const vector<DataPoint>& data,
                           const vector<Cluster>& clusters);
