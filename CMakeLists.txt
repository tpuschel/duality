cmake_minimum_required(VERSION 3.15)

project(duality
    VERSION 0.0.1
    DESCRIPTION "The Duality Programming Language"
    HOMEPAGE_URL "https://duality-lang.org"
    LANGUAGES C
)

add_subdirectory("support")

add_subdirectory("core")

add_subdirectory("syntax")

add_subdirectory("cli")

set(CPACK_PACKAGE_VENDOR "The Duality Programming Language Authors")

set(CPACK_GENERATOR ZIP)

include(CPack)