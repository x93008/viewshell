add_executable(hello_viewshell main.cpp ../common/example_asset_path.cpp)
target_link_libraries(hello_viewshell PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
target_compile_definitions(hello_viewshell PRIVATE EXAMPLE_SOURCE_APP_DIR="${CMAKE_CURRENT_SOURCE_DIR}/app")

install(TARGETS hello_viewshell DESTINATION examples/hello_viewshell)
install(DIRECTORY app/ DESTINATION examples/hello_viewshell/app)
