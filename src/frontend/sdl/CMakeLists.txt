cmake_minimum_required(VERSION 3.20)
project(frontend)

find_package(SDL2 REQUIRED)

add_library(frontend Frontend.cpp Frontend.hpp)
target_include_directories(frontend PUBLIC .)
target_link_libraries(frontend PUBLIC SDL2)