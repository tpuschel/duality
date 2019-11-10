cmake_minimum_required(VERSION 3.15)

project(duality)

if(NOT TARGET support)
    add_subdirectory("support")
endif()

if(NOT TARGET core)
    add_subdirectory("core")
endif()

if(NOT TARGET syntax)
    add_subdirectory("syntax")
endif()

if(NOT TARGET cli)
    add_subdirectory("cli")
endif()

if(NOT TARGET ide)
    add_subdirectory("ide")
endif()
