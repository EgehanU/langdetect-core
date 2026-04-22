cmake_minimum_required(VERSION 3.15)

project(langdetect_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LANGDETECT_BUILD_PYTHON "Build Python bindings" OFF)

# Core library
add_library(langdetect_lib STATIC 
    src/utf8.cpp
    src/detector.cpp
)

set_target_properties(langdetect_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(langdetect_lib PUBLIC include/)

# JSON dependency
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(langdetect_lib PRIVATE nlohmann_json::nlohmann_json)

# CLI
add_executable(langdetect_cli app/langdetect_cli.cpp)
target_link_libraries(langdetect_cli PRIVATE langdetect_lib)

# Tests
add_executable(test_utf8 tests/test_utf8.cpp)
target_link_libraries(test_utf8 PRIVATE langdetect_lib)

add_executable(test_detector tests/test_detector.cpp)
target_link_libraries(test_detector PRIVATE langdetect_lib)

# Benchmark
add_executable(bench_detector benchmarks/bench_detector.cpp)
target_link_libraries(bench_detector PRIVATE langdetect_lib)

# Python module (pybind11)
if(LANGDETECT_BUILD_PYTHON)
    find_package(pybind11 CONFIG REQUIRED)
    pybind11_add_module(langdetect python/bindings.cpp)
    target_link_libraries(langdetect PRIVATE langdetect_lib)
    target_include_directories(langdetect PRIVATE include/)
    install(TARGETS langdetect DESTINATION .)
endif()
