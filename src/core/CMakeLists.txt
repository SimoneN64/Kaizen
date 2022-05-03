cmake_minimum_required(VERSION 3.20)
project(cores CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(gb)
add_library(cores BaseCore.cpp BaseCore.hpp common.hpp)
target_include_directories(cores PUBLIC .)
