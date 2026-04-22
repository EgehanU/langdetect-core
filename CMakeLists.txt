cmake_minimum_required(VERSION 3.15)

project(langdetect-core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(langdetect_lib STATIC 
    src/utf8.cpp
    src/detector.cpp
)

target_include_directories(langdetect_lib PUBLIC include/)

add_executable(langdetect app/langdetect_cli.cpp)
target_link_libraries(langdetect PRIVATE langdetect_lib)

add_executable(test_utf8 tests/test_utf8.cpp)
target_link_libraries(test_utf8 PRIVATE langdetect_lib)

add_executable(test_detector tests/test_detector.cpp)
target_link_libraries(test_detector PRIVATE langdetect_lib)

find_package(nlohmann_json REQUIRED)
target_link_libraries(langdetect_lib PRIVATE nlohmann_json::nlohmann_json)

add_executable(bench_detector benchmarks/bench_detector.cpp)
target_link_libraries(bench_detector PRIVATE langdetect_lib)