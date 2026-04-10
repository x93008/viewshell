cmake_minimum_required(VERSION 3.16)
project(viewshell LANGUAGES CXX)

add_library(viewshell STATIC)
include(cmake/ViewshellDeps.cmake)
target_compile_features(viewshell PUBLIC cxx_std_20)
target_sources(viewshell PRIVATE
  src/viewshell/application.cpp
  src/viewshell/public_types.cpp
)
target_include_directories(viewshell PUBLIC include)

enable_testing()
add_subdirectory(tests)
