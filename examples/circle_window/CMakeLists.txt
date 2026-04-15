add_executable(circle_window main.cpp ../common/example_asset_path.cpp)
target_link_libraries(circle_window PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})

set(CIRCLE_WINDOW_ASSETS
    ${CMAKE_CURRENT_SOURCE_DIR}/app/index.html
    ${CMAKE_CURRENT_SOURCE_DIR}/app/main.js
)
set(CIRCLE_WINDOW_ASSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/app)

set(CIRCLE_WINDOW_ASSET_STAMP ${CMAKE_CURRENT_BINARY_DIR}/circle_window_assets.stamp)
add_custom_command(
    OUTPUT ${CIRCLE_WINDOW_ASSET_STAMP}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CIRCLE_WINDOW_ASSET_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/app ${CIRCLE_WINDOW_ASSET_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${CIRCLE_WINDOW_ASSET_STAMP}
    DEPENDS ${CIRCLE_WINDOW_ASSETS}
)
add_custom_target(circle_window_assets ALL DEPENDS ${CIRCLE_WINDOW_ASSET_STAMP})

add_dependencies(circle_window circle_window_assets)

install(TARGETS circle_window DESTINATION examples/circle_window)
install(DIRECTORY app/ DESTINATION examples/circle_window/app)
