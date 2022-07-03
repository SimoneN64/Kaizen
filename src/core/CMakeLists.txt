cmake_minimum_required(VERSION 3.20)
project(cores CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(n64)

add_library(cores
  Scheduler.cpp
  Scheduler.hpp
  common.hpp
  util.hpp
  Audio.hpp
  Audio.cpp)
target_include_directories(cores PUBLIC . ../../external)
target_link_libraries(cores PUBLIC n64)
