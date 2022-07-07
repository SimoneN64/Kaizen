cmake_minimum_required(VERSION 3.20)
project(cores CXX)

add_subdirectory(n64)

find_package(fmt REQUIRED)

add_library(cores
  common.hpp
  util.hpp
  BaseCore.hpp)

target_link_libraries(cores PUBLIC fmt::fmt n64)
target_include_directories(cores PUBLIC . ../../external)