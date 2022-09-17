cmake_minimum_required(VERSION 3.20)
project(imgui)

find_package(SDL2 REQUIRED)

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
	target_compile_options(imgui PUBLIC -mwindows)
	set(LIBRARIES -static z stdc++ user32 gdi32 winmm Imm32 ole32 oleaut32 shell32 setupapi version uuid)
else()
	set(LIBRARIES )
endif()

target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS} imgui imgui/backends)
target_link_libraries(imgui PUBLIC SDL2main SDL2 ${LIBRARIES} )