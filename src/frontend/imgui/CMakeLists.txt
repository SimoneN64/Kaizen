cmake_minimum_required(VERSION 3.20)
project(frontend-imgui)

add_subdirectory(../../../external/imgui temp)
add_subdirectory(../../../external/nativefiledialog-extended temp1)

find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)

add_library(frontend-imgui STATIC
  Window.cpp
  Window.hpp
  widgets.cpp
  debugger.hpp
  debugger.cpp
  debugger/cpu/decode.hpp
  debugger/cop0/decode.hpp
  debugger/fpu/decode.hpp
  debugger/rsp/decode.hpp
  debugger/debugger_common.hpp)

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
target_link_libraries(frontend-imgui PUBLIC ws2_32 SDL2main SDL2 imgui nfd fmt)