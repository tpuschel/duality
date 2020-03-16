add_library(duality-dap
    "src/dap.c"
)

set_target_properties(duality-dap PROPERTIES C_VISIBILITY_PRESET hidden)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(duality-dap
        PRIVATE DY_DAP_EXPORT
        INTERFACE DY_DAP_IMPORT
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(duality-dap
        PRIVATE _GNU_SOURCE
    )
endif()

target_include_directories(duality-dap PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(duality-dap
    PUBLIC duality-support
)

install(
    TARGETS duality-dap
    EXPORT duality-dap-config.cmake
)

install(
    EXPORT duality-dap-config.cmake
    DESTINATION cmake
)

install(
    DIRECTORY include/
    TYPE INCLUDE
)