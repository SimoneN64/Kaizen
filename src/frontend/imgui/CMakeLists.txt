cmake_minimum_required(VERSION 3.20)
project(frontend-imgui)

add_subdirectory(../../../external/imgui temp)
add_subdirectory(../../../external/nativefiledialog-extended temp1)

add_library(frontend-imgui
  Window.cpp
  Window.hpp)

target_include_directories(frontend-imgui PUBLIC . ../../cores ../../../external)
target_link_libraries(frontend-imgui PUBLIC imgui nfd)