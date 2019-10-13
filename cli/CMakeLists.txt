add_executable(dy
    "src/main.c"
)

target_link_libraries(dy
    PRIVATE duality-support
    PRIVATE duality-core
    PRIVATE duality-syntax
)

install(
    TARGETS dy
    EXPORT dy-config.cmake
)

install(
    EXPORT dy-config.cmake
    DESTINATION cmake
)