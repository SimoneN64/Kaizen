cmake_minimum_required(VERSION 3.20)
project(n64)

add_subdirectory(core)

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)

add_library(n64
  Core.cpp
  Core.hpp
  memory_regions.hpp)

target_link_libraries(n64 PUBLIC core fmt SDL2main SDL2)
target_include_directories(n64 PUBLIC
  .
  ..
  ../frontend/imgui/debugger
  ../../external
  ../../external/capstone/include
  ../../external/imgui/imgui
  ../../external/imgui/imgui/backends)
