add_executable(duality
    "src/main.c"
)

target_link_libraries(duality
    PRIVATE duality-support
    PRIVATE duality-core
    PRIVATE duality-syntax
    PRIVATE duality-lsp
    PRIVATE duality-dap
)

install(
    TARGETS duality
    EXPORT duality-config.cmake
)

install(
    EXPORT duality-config.cmake
    DESTINATION cmake
)