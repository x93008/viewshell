add_executable(linux_x11_smoke main.cpp)
target_link_libraries(linux_x11_smoke PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})

install(TARGETS linux_x11_smoke
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    BUNDLE DESTINATION bin
)
install(DIRECTORY app/ DESTINATION share/linux_x11_smoke/app)
