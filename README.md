# MiniCluster

An explainable, dependency-free C++17 implementation of 2D K-Means clustering. It turns a simple algorithm assignment into a small analysis tool: load a CSV, choose an initialization strategy, inspect cluster quality, spot unusual records, and export assignments for further work.

## What makes it stand out

- **Strategy pattern for centroid selection:** swap between random seeding and K-Means++ without changing the core algorithm.
- **Reproducible experiments:** every run accepts a fixed random seed, making results easy to defend and repeat.
- **Robust convergence:** supports a tolerance threshold, iteration cap, and recovery from empty clusters.
- **Useful diagnostics:** reports inertia, silhouette score, cluster compactness, and statistically unusual members.
- **Terminal visualization:** an optional ASCII map gives a quick, dependency-free view of the discovered groups.
- **Portable data workflow:** reads CSV/TSV/semicolon-delimited two-column numeric data and exports every point's cluster label.

## Build and test

```bash
make
make test
```

## Run it

Use the deterministic synthetic customer demo:

```bash
./build/minicluster --demo --k 3 --seed 42 --map --export cluster_assignments.csv
```

Use the included 930-record Mall Customers-style dataset. It deliberately has overlapping, unevenly spread customer profiles and unusual records, so the clusters are more realistic than perfectly separated synthetic groups. The first two numeric columns are used; a header row is allowed.

```bash
./build/minicluster --input data/mall_customers_large.csv --k 6 --init kmeans++ --map
```

Create a presentation-ready scatter plot. It opens in any browser because it is an SVG file.

```bash
./build/minicluster --input data/mall_customers_large.csv --k 6 --graph cluster_plot.svg
```

Compare initialization strategies fairly with the same seed:

```bash
./build/minicluster --input data/mall_customers_large.csv --k 6 --init random --seed 42
./build/minicluster --input data/mall_customers_large.csv --k 6 --init kmeans++ --seed 42
```

## Reading the report

- **Inertia:** total squared distance from each point to its assigned centroid. Lower is more compact for a fixed `k`.
- **Silhouette:** ranges from `-1` to `1`; larger values suggest better-separated clusters.
- **Average/max radius:** shows each cluster's spread around its centroid.
- **Outlier candidates:** members whose distance from their own centroid is at least 2.5 standard deviations above their cluster average.
- **SVG graph:** color-coded customer points, centroid markers, axes, and cluster labels for a submission-ready visual.

## Project layout

```text
include/       Public domain and algorithm interfaces
src/           K-Means engine, initialization strategies, analysis, CSV I/O, CLI
tests/         Fast deterministic regression test
data/          Ready-to-run customer-style input sample
```

This project intentionally uses only the C++ standard library, so it compiles cleanly on any C++17 toolchain.
