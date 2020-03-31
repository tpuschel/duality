add_library(duality-syntax
    "src/ast_to_core.c"
    "src/parser.c"
    "src/ast.c"
)

set_target_properties(duality-syntax PROPERTIES C_VISIBILITY_PRESET hidden)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(duality-syntax
        PRIVATE DY_SYNTAX_EXPORT
        INTERFACE DY_SYNTAX_IMPORT
    )
endif()

target_include_directories(duality-syntax PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(duality-syntax
    PUBLIC duality-support
    PUBLIC duality-core
)

install(
    TARGETS duality-syntax
    EXPORT duality-syntax-config.cmake
)

install(
    EXPORT duality-syntax-config.cmake
    DESTINATION cmake
)

install(
    DIRECTORY include/
    TYPE INCLUDE
)