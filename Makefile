CXX := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
BUILD_DIR := build
CORE_SOURCES := src/analytics.cpp src/chart.cpp src/data_loader.cpp src/data_point.cpp src/distance_metric.cpp src/exporter.cpp src/initializers.cpp src/interactive_cli.cpp src/kmeans.cpp src/pipeline.cpp

.PHONY: all test run-demo clean

all: $(BUILD_DIR)/minicluster

$(BUILD_DIR)/minicluster: $(CORE_SOURCES) src/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CORE_SOURCES) src/main.cpp -o $@

$(BUILD_DIR)/minicluster_tests: $(CORE_SOURCES) tests/test_kmeans.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CORE_SOURCES) tests/test_kmeans.cpp -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(BUILD_DIR)/minicluster_tests
	./$(BUILD_DIR)/minicluster_tests

run-demo: $(BUILD_DIR)/minicluster
	./$(BUILD_DIR)/minicluster --demo --k 3 --seed 42 --graph demo_clusters

clean:
	rm -rf $(BUILD_DIR)
