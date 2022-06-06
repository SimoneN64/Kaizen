cmake_minimum_required(VERSION 3.20)
project(frontend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(frontend Frontend.hpp Frontend.cpp)

target_compile_definitions(frontend PUBLIC FRONTEND_QT)
target_include_directories(frontend PUBLIC .)
target_link_libraries(frontend PUBLIC Qt5::Widgets)
