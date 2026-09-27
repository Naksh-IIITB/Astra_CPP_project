# Astra C++ Project

## K-Means MiniCluster

This project implements K-Means clustering for unlabeled 2D data in standard C++17.

### Features

- Euclidean-distance assignment of points to clusters
- Centroid recomputation until convergence or a maximum iteration count
- K-Means++ centroid initialization
- Empty-cluster recovery by selecting a new data point
- Final centroid coordinates, cluster sizes, and members are printed
- Input validation for invalid `k`, empty data, and inconsistent dimensions

The implementation also includes a reusable `RandomInitialiser` alongside the K-Means++ initializer. The sample program uses K-Means++ on a small set of generated-style 2D customer points and uses `k = 3`.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -O2 main.cpp -o kmeans
./kmeans
```

A fixed random seed is used so that the sample output is reproducible.

## Algorithm

1. Select `k` initial centroids using K-Means++.
2. Assign each point to its nearest centroid using squared Euclidean distance.
3. Replace each centroid with the mean of its assigned points.
4. Repeat until centroids stop moving or 100 iterations have elapsed.
5. Print each centroid, its cluster size, and its members.

The implementation is `O(i * n * k * d)`, where `i` is the number of iterations, `n` is the number of points, `k` is the number of clusters, and `d` is the number of dimensions.

## Design notes

- Squared distance is used because it gives the same nearest point as Euclidean distance without the unnecessary square root.
- K-Means++ generally gives better starting positions than selecting completely random centroids.
- K-Means can produce different clusters for different seeds because initialization affects convergence.
- Real datasets should generally be feature-scaled before clustering when dimensions use different units.
