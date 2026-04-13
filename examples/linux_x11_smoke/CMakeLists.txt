add_executable(linux_x11_smoke main.cpp)
target_link_libraries(linux_x11_smoke PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})

install(TARGETS linux_x11_smoke RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY app/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/linux_x11_smoke/app)
