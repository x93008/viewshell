add_executable(circle_window main.cpp)
target_link_libraries(circle_window PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})

install(TARGETS circle_window DESTINATION examples/circle_window)
install(DIRECTORY app/ DESTINATION examples/circle_window/app)
