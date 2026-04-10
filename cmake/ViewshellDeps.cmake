find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/.conan")
find_package(nlohmann_json REQUIRED)

target_include_directories(viewshell PUBLIC third_party/tl_expected/include)
target_link_libraries(viewshell PUBLIC nlohmann_json::nlohmann_json)
