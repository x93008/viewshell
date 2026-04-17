add_executable(custom_tray_demo main.cpp ../common/example_asset_path.cpp)
target_link_libraries(custom_tray_demo PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_definitions(custom_tray_demo PRIVATE EXAMPLE_SOURCE_APP_DIR="${CMAKE_CURRENT_SOURCE_DIR}/app")

install(TARGETS custom_tray_demo DESTINATION examples/custom_tray_demo)
install(DIRECTORY app/ DESTINATION examples/custom_tray_demo/app)
