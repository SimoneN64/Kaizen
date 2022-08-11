cmake_minimum_required(VERSION 3.20)
project(frontend-imgui)

add_subdirectory(../../../external/imgui temp)
add_subdirectory(../../../external/nativefiledialog-extended temp1)

find_package(SDL2 REQUIRED)

add_library(frontend-imgui
  Window.cpp
  Window.hpp)

target_include_directories(frontend-imgui PUBLIC
  .
  ../..
  ../../n64
  ../../n64/core
  ../../n64/core/cpu
  ../../n64/core/cpu/registers
  ../../../external
  ../../../external/parallel-rdp/parallel-rdp-standalone
  ../../../external/parallel-rdp/parallel-rdp-standalone/vulkan
  ../../../external/parallel-rdp/parallel-rdp-standalone/util
  ../../../external/parallel-rdp/parallel-rdp-standalone/volk)
target_link_libraries(frontend-imgui PUBLIC SDL2 imgui nfd)