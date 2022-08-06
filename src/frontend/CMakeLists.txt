cmake_minimum_required(VERSION 3.20)
project(frontend)

add_subdirectory(imgui)

add_library(frontend
  App.hpp
  App.cpp)

target_include_directories(frontend PUBLIC
  .
  ..
  ../../external
  ../../external/parallel-rdp
  ../../external/parallel-rdp/parallel-rdp-standalone/vulkan
  ../../external/parallel-rdp/parallel-rdp-standalone/util
  ../../external/parallel-rdp/parallel-rdp-standalone/volk
  ../n64
  ../n64/core
  ../n64/core/cpu
  ../n64/core/cpu/registers)
target_link_libraries(frontend PUBLIC frontend-imgui)
