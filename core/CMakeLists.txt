add_library(duality-core
    "src/are_equal.c"
    "src/eval.c"
    "src/check.c"
    "src/substitute.c"
    "src/is_subtype.c"
    "src/ternary.c"
    "src/type_of.c"
    "src/core.c"
)

set_target_properties(duality-core PROPERTIES C_VISIBILITY_PRESET hidden)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(duality-core
        PRIVATE DY_CORE_EXPORT
        INTERFACE DY_CORE_IMPORT
    )
endif()

target_include_directories(duality-core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(duality-core PUBLIC duality-support)

install(
    TARGETS duality-core
    EXPORT duality-core-config.cmake
)

install(
    EXPORT duality-core-config.cmake
    DESTINATION cmake
)

install(
    DIRECTORY include/
    TYPE INCLUDE
)