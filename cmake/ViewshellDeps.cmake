find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/.conan")
find_package(nlohmann_json REQUIRED)

target_include_directories(viewshell PUBLIC third_party/tl_expected/include)
target_link_libraries(viewshell PUBLIC nlohmann_json::nlohmann_json)

pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(X11 REQUIRED x11)
target_include_directories(viewshell PRIVATE ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
target_link_libraries(viewshell PRIVATE ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_options(viewshell PRIVATE ${GTK3_CFLAGS_OTHER} ${X11_CFLAGS_OTHER})
