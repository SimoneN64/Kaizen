cmake_minimum_required(VERSION 3.20)
project(frontend-imgui)

add_subdirectory(../../../external/imgui temp)
add_subdirectory(../../../external/nativefiledialog-extended temp1)
SET(NFD_PORTAL ON CACHE BOOL "Use dbus for native file dialog instead of gtk")

find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(frontend-imgui STATIC
  Window.cpp
  Window.hpp)

target_include_directories(frontend-imgui PUBLIC
  .
  debugger
  ../..
  ../../n64
  ../../n64/core
  ../../n64/core/cpu
  ../../n64/core/cpu/registers
  ../../../external
  ../../../external/capstone/include
  ../../../external/parallel-rdp/parallel-rdp-standalone
  ../../../external/parallel-rdp/parallel-rdp-standalone/vulkan
  ../../../external/parallel-rdp/parallel-rdp-standalone/util
  ../../../external/parallel-rdp/parallel-rdp-standalone/volk)
  
if(WIN32)
	target_compile_options(frontend-imgui PUBLIC -mwindows)
	set(LIBRARIES -static z stdc++ user32 gdi32 winmm Imm32 ole32 oleaut32 shell32 setupapi version uuid)
else()
	set(LIBRARIES )
endif()

target_link_libraries(frontend-imgui PUBLIC nlohmann_json::nlohmann_json SDL2main SDL2 ${LIBRARIES} imgui nfd fmt)