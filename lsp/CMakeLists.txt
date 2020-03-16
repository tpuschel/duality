add_library(duality-lsp
    "src/lsp.c"
    "src/server.c"
)

set_target_properties(duality-lsp PROPERTIES C_VISIBILITY_PRESET hidden)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(duality-lsp
        PRIVATE DY_LSP_EXPORT
        INTERFACE DY_LSP_IMPORT
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(duality-lsp
        PRIVATE _GNU_SOURCE
    )
endif()

target_include_directories(duality-lsp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(duality-lsp
    PUBLIC duality-support
    PUBLIC duality-syntax
    PUBLIC duality-core
)

install(
    TARGETS duality-lsp
    EXPORT duality-lsp-config.cmake
)

install(
    EXPORT duality-lsp-config.cmake
    DESTINATION cmake
)

install(
    DIRECTORY include/
    TYPE INCLUDE
)