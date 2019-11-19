cmake_minimum_required(VERSION 3.15)

project(duality)

option(DY_BUILD_IDE "Build the IDE written in Qt" ON)

add_subdirectory("support")

add_subdirectory("core")

add_subdirectory("syntax")

add_subdirectory("cli")

if (DY_BUILD_IDE)
    add_subdirectory("ide")
endif()
