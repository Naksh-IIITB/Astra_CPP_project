#pragma once

#include "analytics.h"
#include "data_loader.h"
#include "kmeans.h"

#include <istream>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct Session {
    Session(std::istream& inputStream, std::ostream& outputStream) : input(inputStream), output(outputStream) {}
    std::istream& input;
    std::ostream& output;
    std::vector<DataPoint> data;
    std::string dataDescription;
    std::size_t k = 6;
    std::string initializerName = "kmeans++";
    std::string metricName = "euclidean";
    unsigned int seed = 42;
    std::size_t maxIterations = 200;
    std::unique_ptr<KMeans> lastModel;
    double lastInertia = 0.0;
    double lastSilhouette = 0.0;
    bool running = true;
};

class MenuAction {
public:
    virtual ~MenuAction() = default;
    virtual std::string label() const = 0;
    virtual void run(Session& session) = 0;
};

class InteractiveCli {
public:
    explicit InteractiveCli(std::istream& input = std::cin, std::ostream& output = std::cout);
    void run();

private:
    Session session_;
    std::vector<std::unique_ptr<MenuAction>> actions_;
};
