cmake_minimum_required(VERSION 3.16)
project(viewshell LANGUAGES CXX)

add_library(viewshell STATIC)
include(cmake/ViewshellDeps.cmake)
target_compile_features(viewshell PUBLIC cxx_std_20)
target_sources(viewshell PRIVATE
  src/viewshell/application.cpp
  src/viewshell/public_types.cpp
  src/viewshell/window_handle.cpp
  src/viewshell/bridge_handle.cpp
  src/platform/linux_x11/window_driver.cpp
  src/platform/linux_x11/kernel_resolver.cpp
  src/platform/linux_x11/resource_protocol.cpp
  src/platform/linux_x11/webview_driver.cpp
  src/platform/linux_x11/bridge_driver.cpp
)
target_include_directories(viewshell PUBLIC include)

enable_testing()
add_subdirectory(tests)
