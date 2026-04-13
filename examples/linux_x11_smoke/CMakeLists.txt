add_executable(linux_x11_smoke main.cpp)
target_link_libraries(linux_x11_smoke PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})

install(TARGETS linux_x11_smoke
    RUNTIME DESTINATION examples/linux_x11_smoke
    LIBRARY DESTINATION examples/linux_x11_smoke
    ARCHIVE DESTINATION examples/linux_x11_smoke
    BUNDLE DESTINATION examples/linux_x11_smoke
)
install(DIRECTORY app/ DESTINATION examples/linux_x11_smoke/app)
