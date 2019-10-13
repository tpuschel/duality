cmake_minimum_required(VERSION 2.4)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0063 NEW)

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