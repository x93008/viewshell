add_executable(hover_test main.cpp ../common/example_asset_path.cpp)
target_link_libraries(hover_test PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_definitions(hover_test PRIVATE EXAMPLE_SOURCE_APP_DIR="${CMAKE_CURRENT_SOURCE_DIR}/app")

install(TARGETS hover_test DESTINATION examples/hover_test)
install(DIRECTORY app/ DESTINATION examples/hover_test/app)
