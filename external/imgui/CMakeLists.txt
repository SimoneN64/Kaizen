cmake_minimum_required(VERSION 3.20)
project(imgui)

find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_library(imgui
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_internal.h
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui.cpp
  imgui/imgui.h
  imgui/imconfig.h
  imgui/backends/imgui_impl_sdl.h
  imgui/backends/imgui_impl_sdl.cpp
  imgui/backends/imgui_impl_vulkan.h
  imgui/backends/imgui_impl_vulkan.cpp)


if(WIN32)
  add_compile_definitions(NOMINMAX _CRT_SECURE_NO_WARNINGS)
  add_compile_options(/EHa)
endif()

target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS} imgui imgui/backends)
target_link_libraries(imgui PUBLIC SDL2::SDL2main SDL2::SDL2 Vulkan::Vulkan)