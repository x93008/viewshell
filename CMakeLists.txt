cmake_minimum_required(VERSION 3.16)
project(viewshell LANGUAGES CXX)

find_package(nlohmann_json REQUIRED)
find_package(tl-expected REQUIRED)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)

enable_testing()
