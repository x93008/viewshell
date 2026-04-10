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
