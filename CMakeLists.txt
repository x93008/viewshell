cmake_minimum_required(VERSION 3.16)
project(viewshell LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(CREATE_LINK "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}/compile_commands.json" SYMBOLIC RESULT _create_link_result)
if(NOT _create_link_result EQUAL 0)
    message(WARNING "Failed to create compile_commands.json symlink: ${_create_link_result}")
endif()

find_package(nlohmann_json REQUIRED)
find_package(tl-expected REQUIRED)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)

enable_testing()
