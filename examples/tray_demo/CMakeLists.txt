add_executable(tray_demo main.cpp ../common/example_asset_path.cpp)
target_link_libraries(tray_demo PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_definitions(tray_demo PRIVATE EXAMPLE_SOURCE_APP_DIR="${CMAKE_CURRENT_SOURCE_DIR}/app")

install(TARGETS tray_demo DESTINATION examples/tray_demo)
install(DIRECTORY app/ DESTINATION examples/tray_demo/app)
