cmake_minimum_required(VERSION 3.20)
project(frontend)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(frontend Frontend.hpp)
target_include_directories(frontend PUBLIC .)
target_link_libraries(frontend PUBLIC Qt5::Widgets)
