#pragma once

#include "kmeans.h"

#include <string>
#include <vector>

using namespace std;

void writeClusterSvg(const string& filename,
                     const vector<DataPoint>& data,
                     const vector<Cluster>& clusters);

void writeClusterPng(const string& filename,
                     const vector<DataPoint>& data,
                     const vector<Cluster>& clusters);
