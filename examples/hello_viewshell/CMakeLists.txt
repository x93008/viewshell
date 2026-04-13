add_executable(hello_viewshell main.cpp)
target_link_libraries(hello_viewshell PRIVATE viewshell nlohmann_json::nlohmann_json ${GTK3_LIBRARIES} ${X11_LIBRARIES})

set(HELLO_VIEWSHELL_ASSETS
    ${CMAKE_CURRENT_SOURCE_DIR}/app/index.html
    ${CMAKE_CURRENT_SOURCE_DIR}/app/main.js
)
set(HELLO_VIEWSHELL_ASSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/app)

set(HELLO_VIEWSHELL_ASSET_STAMP ${CMAKE_CURRENT_BINARY_DIR}/hello_viewshell_assets.stamp)
add_custom_command(
    OUTPUT ${HELLO_VIEWSHELL_ASSET_STAMP}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${HELLO_VIEWSHELL_ASSET_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/app ${HELLO_VIEWSHELL_ASSET_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${HELLO_VIEWSHELL_ASSET_STAMP}
    DEPENDS ${HELLO_VIEWSHELL_ASSETS}
)
add_custom_target(hello_viewshell_assets ALL DEPENDS ${HELLO_VIEWSHELL_ASSET_STAMP})

add_dependencies(hello_viewshell hello_viewshell_assets)

install(TARGETS hello_viewshell DESTINATION examples/hello_viewshell)
install(DIRECTORY app/ DESTINATION examples/hello_viewshell/app)
