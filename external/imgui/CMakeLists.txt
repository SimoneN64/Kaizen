cmake_minimum_required(VERSION 3.20)
project(imgui)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED FATAL_ERROR)

add_library(imgui
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_internal.h
  imgui_tables.cpp
  imgui_widgets.cpp
  imgui.cpp
  imgui.h
  imconfig.h
  backends/imgui_impl_sdl.h
  backends/imgui_impl_sdl.cpp
  backends/imgui_impl_vulkan.h
  backends/imgui_impl_vulkan.cpp)

target_link_libraries(imgui PUBLIC SDL2 ${Vulkan_LIBRARY})
target_include_directories(imgui PUBLIC . backends)