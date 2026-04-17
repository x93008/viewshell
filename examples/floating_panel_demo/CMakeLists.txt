add_executable(floating_panel_demo main.cpp ../common/example_asset_path.cpp)
target_link_libraries(floating_panel_demo PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_definitions(floating_panel_demo PRIVATE EXAMPLE_SOURCE_APP_DIR="${CMAKE_CURRENT_SOURCE_DIR}/app")

install(TARGETS floating_panel_demo DESTINATION examples/floating_panel_demo)
install(DIRECTORY app/ DESTINATION examples/floating_panel_demo/app)
