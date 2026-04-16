add_executable(circle_window main.cpp ../common/example_asset_path.cpp)
target_link_libraries(circle_window PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_definitions(circle_window PRIVATE EXAMPLE_SOURCE_APP_DIR="${CMAKE_CURRENT_SOURCE_DIR}/app")

install(TARGETS circle_window DESTINATION examples/circle_window)
install(DIRECTORY app/ DESTINATION examples/circle_window/app)
