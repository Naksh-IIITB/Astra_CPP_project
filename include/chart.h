#pragma once

#include "kmeans.h"

#include <string>
#include <vector>

void writeClusterSvg(const string& filename,
                     const vector<DataPoint>& data,
                     const vector<Cluster>& clusters);

void writeClusterPng(const string& filename,
                     const vector<DataPoint>& data,
                     const vector<Cluster>& clusters);
