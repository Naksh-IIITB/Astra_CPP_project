cmake_minimum_required(VERSION 3.16)
project(MiniCluster VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(minicluster_core
    src/analytics.cpp
    src/csv_io.cpp
    src/data_point.cpp
    src/initializers.cpp
    src/kmeans.cpp)
target_include_directories(minicluster_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(minicluster_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(minicluster src/main.cpp)
target_link_libraries(minicluster PRIVATE minicluster_core)
target_compile_options(minicluster PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
    add_executable(minicluster_tests tests/test_kmeans.cpp)
    target_link_libraries(minicluster_tests PRIVATE minicluster_core)
    add_test(NAME minicluster_tests COMMAND minicluster_tests)
endif()
