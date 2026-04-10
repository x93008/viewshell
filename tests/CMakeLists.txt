include(GoogleTest)

add_executable(application_create_test unit/application_create_test.cpp)
target_link_libraries(application_create_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(application_create_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(application_lifecycle_test unit/application_lifecycle_test.cpp)
target_link_libraries(application_lifecycle_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(application_lifecycle_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(window_lifecycle_test unit/window_lifecycle_test.cpp unit/runtime_test_hooks.cpp)
target_link_libraries(window_lifecycle_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(window_lifecycle_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(window_driver_smoke_test integration/window_driver_smoke_test.cpp)
target_include_directories(window_driver_smoke_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
target_compile_options(window_driver_smoke_test PRIVATE ${GTK3_CFLAGS_OTHER})
target_link_libraries(window_driver_smoke_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
gtest_discover_tests(window_driver_smoke_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(kernel_resolver_test unit/kernel_resolver_test.cpp)
target_include_directories(kernel_resolver_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(kernel_resolver_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(kernel_resolver_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(resource_protocol_test unit/resource_protocol_test.cpp)
target_include_directories(resource_protocol_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(resource_protocol_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(resource_protocol_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(webview_driver_test unit/webview_driver_test.cpp)
target_include_directories(webview_driver_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${GTK3_INCLUDE_DIRS})
target_compile_options(webview_driver_test PRIVATE ${GTK3_CFLAGS_OTHER})
target_link_libraries(webview_driver_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json ${GTK3_LIBRARIES})
gtest_discover_tests(webview_driver_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(local_app_smoke_test integration/local_app_smoke_test.cpp integration/test_app_harness.cpp)
target_include_directories(local_app_smoke_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
target_compile_options(local_app_smoke_test PRIVATE ${GTK3_CFLAGS_OTHER})
target_link_libraries(local_app_smoke_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
gtest_discover_tests(local_app_smoke_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(bridge_driver_test unit/bridge_driver_test.cpp unit/bridge_driver_test_hooks.cpp)
target_include_directories(bridge_driver_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${GTK3_INCLUDE_DIRS})
target_compile_options(bridge_driver_test PRIVATE ${GTK3_CFLAGS_OTHER})
target_link_libraries(bridge_driver_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json ${GTK3_LIBRARIES})
gtest_discover_tests(bridge_driver_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(trust_gate_test unit/trust_gate_test.cpp)
target_include_directories(trust_gate_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(trust_gate_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(trust_gate_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(remote_untrusted_test integration/remote_untrusted_test.cpp integration/test_app_harness.cpp)
target_include_directories(remote_untrusted_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
target_compile_options(remote_untrusted_test PRIVATE ${GTK3_CFLAGS_OTHER})
target_link_libraries(remote_untrusted_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
gtest_discover_tests(remote_untrusted_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(request_tracker_test unit/request_tracker_test.cpp)
target_include_directories(request_tracker_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(request_tracker_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(request_tracker_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(builtin_dispatcher_test unit/builtin_dispatcher_test.cpp)
target_include_directories(builtin_dispatcher_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(builtin_dispatcher_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(builtin_dispatcher_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(invoke_bus_test unit/invoke_bus_test.cpp)
target_include_directories(invoke_bus_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(invoke_bus_test PRIVATE viewshell GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(invoke_bus_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
