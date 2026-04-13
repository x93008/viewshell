add_library(viewshell STATIC)
target_compile_features(viewshell PUBLIC cxx_std_17)
target_include_directories(viewshell PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(viewshell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(viewshell PRIVATE
  viewshell/application.cpp
  viewshell/public_types.cpp
  viewshell/window_handle.cpp
  viewshell/bridge_handle.cpp
  platform/linux_x11/window_driver.cpp
  platform/linux_x11/kernel_resolver.cpp
  platform/linux_x11/resource_protocol.cpp
  platform/linux_x11/webview_driver.cpp
  platform/linux_x11/bridge_driver.cpp
  platform/linux_x11/trust_gate.cpp
  platform/linux_x11/request_tracker.cpp
  platform/linux_x11/builtin_dispatcher.cpp
  platform/linux_x11/invoke_bus.cpp
)

include(${CMAKE_SOURCE_DIR}/cmake/ViewshellDeps.cmake)

install(TARGETS viewshell
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/viewshell
    DESTINATION include
)
