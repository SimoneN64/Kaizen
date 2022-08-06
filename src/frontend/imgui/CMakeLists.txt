cmake_minimum_required(VERSION 3.20)
project(frontend-imgui)

add_subdirectory(../../../external/imgui temp)
add_subdirectory(../../../external/nativefiledialog-extended temp1)

add_library(frontend-imgui
  Window.cpp
  Window.hpp)

target_include_directories(frontend-imgui PUBLIC
  .
  ../../n64
  ../../../external
  ../../../external/parallel-rdp/parallel-rdp-standalone/vulkan
  ../../../external/parallel-rdp/parallel-rdp-standalone/util
  ../../../external/parallel-rdp/parallel-rdp-standalone/volk)
target_link_libraries(frontend-imgui PUBLIC imgui nfd)