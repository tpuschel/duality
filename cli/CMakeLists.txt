add_executable(duality
    "src/main.c"
    "src/lsp_server.c"
    "src/utf8_to_json.c"
    "src/json_to_utf8.c"
)

target_link_libraries(duality
    PRIVATE duality-support
    PRIVATE duality-core
    PRIVATE duality-syntax
)

install(
    TARGETS duality
    EXPORT duality-config.cmake
)

install(
    EXPORT duality-config.cmake
    DESTINATION cmake
)