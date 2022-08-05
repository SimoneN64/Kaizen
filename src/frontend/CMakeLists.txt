cmake_minimum_required(VERSION 3.20)
project(frontend)

add_subdirectory(imgui)

add_library(frontend
  App.hpp
  App.cpp)

target_include_directories(frontend PUBLIC
  .
  ../../external
  ../../external/parallel-rdp
  ../../external/parallel-rdp/parallel-rdp-standalone/vulkan
  ../../external/parallel-rdp/parallel-rdp-standalone/util
  ../../external/parallel-rdp/parallel-rdp-standalone/volk
  ../cores
  ../cores/n64)
target_link_libraries(frontend PUBLIC frontend-imgui)
