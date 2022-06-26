cmake_minimum_required(VERSION 3.20)
project(natsukashii-qt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(natsukashii-qt Frontend.hpp Frontend.cpp main.cpp)

target_include_directories(natsukashii-qt PRIVATE . ../../core ../../core/gb ../../core/n64)
target_link_libraries(natsukashii-qt PRIVATE cores Qt5::Widgets)
