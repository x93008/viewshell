add_library(viewshell STATIC)
target_compile_features(viewshell PUBLIC cxx_std_17)
target_include_directories(viewshell PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(viewshell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(viewshell PRIVATE
  runtime/backend_factory.cpp
  viewshell/application.cpp
  viewshell/public_types.cpp
  viewshell/window_handle.cpp
  viewshell/bridge_handle.cpp
)

if(WIN32)
  target_sources(viewshell PRIVATE
    runtime/win32_backend_runtime.cpp
    platform/windows/win32_window_host.cpp
  )
else()
  target_sources(viewshell PRIVATE
    runtime/x11_backend_runtime.cpp
    platform/x11/x11_window_host.cpp
    window/x11_window_driver.cpp
    webview/engine_resolver.cpp
    webview/resource_protocol.cpp
    webview/x11_webview_driver.cpp
    bridge/x11_bridge_driver.cpp
    bridge/trust_gate.cpp
    bridge/request_tracker.cpp
    bridge/builtin_dispatcher.cpp
    bridge/invoke_bus.cpp
  )
endif()

include(${CMAKE_SOURCE_DIR}/cmake/ViewshellDeps.cmake)

install(TARGETS viewshell
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/viewshell
    DESTINATION include
)
