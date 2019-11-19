set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(duality-ide
    "src/main.cpp"
    "src/App.cpp"
)

target_link_libraries(duality-ide
    PRIVATE duality-support
    PRIVATE duality-core
    PRIVATE duality-syntax
    PRIVATE Qt5::Widgets
)

install(
    TARGETS duality-ide
    EXPORT duality-ide-config.cmake
)

install(
    EXPORT duality-ide-config.cmake
    DESTINATION cmake
)
