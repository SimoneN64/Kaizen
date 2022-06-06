cmake_minimum_required(VERSION 3.20)
project(frontend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(SDL2 REQUIRED)

add_library(frontend Frontend.cpp Frontend.hpp)
target_compile_definitions(frontend PUBLIC FRONTEND_SDL)
target_include_directories(frontend PUBLIC . ../../core ../../core/gb ../../core/n64)
target_link_libraries(frontend PUBLIC SDL2)
