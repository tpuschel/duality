add_library(duality-support
    "src/array.c"
    "src/assert.c"
    "src/string.c"
    "src/overflow.c"
    "src/allocator.c"
)

set_target_properties(duality-support PROPERTIES C_VISIBILITY_PRESET hidden)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(duality-support
        PRIVATE DY_SUPPORT_EXPORT
        INTERFACE DY_SUPPORT_IMPORT
    )
endif()

target_include_directories(duality-support PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(
    TARGETS duality-support
    EXPORT duality-support-config.cmake
)

install(
    EXPORT duality-support-config.cmake
    DESTINATION cmake
)

install(
    DIRECTORY include/
    TYPE INCLUDE
)