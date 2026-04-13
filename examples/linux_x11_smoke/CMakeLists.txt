add_executable(linux_x11_smoke main.cpp)
target_link_libraries(linux_x11_smoke PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})
