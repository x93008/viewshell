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
  src/platform/linux_x11/trust_gate.cpp
  src/platform/linux_x11/request_tracker.cpp
  src/platform/linux_x11/builtin_dispatcher.cpp
  src/platform/linux_x11/invoke_bus.cpp
)
target_include_directories(viewshell PUBLIC include)

enable_testing()
add_subdirectory(tests)

add_executable(linux_x11_smoke examples/linux_x11_smoke/main.cpp)
target_link_libraries(linux_x11_smoke PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
