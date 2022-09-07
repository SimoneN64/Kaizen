cmake_minimum_required(VERSION 3.20)
project(core)

add_subdirectory(cpu)

add_subdirectory(../../../external/parallel-rdp temp)
add_subdirectory(../../../external/capstone temp1)

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)

add_library(core
  Cpu.hpp
  Cpu.cpp
  Mem.cpp
  Mem.hpp
  RDP.cpp
  RDP.hpp
  Audio.cpp
  Audio.hpp
  mmio/AI.cpp
  mmio/AI.hpp
  mmio/Interrupt.cpp
  mmio/Interrupt.hpp
  mmio/MI.cpp
  mmio/MI.hpp
  mmio/PI.cpp
  mmio/PI.hpp
  mmio/PIF.cpp
  mmio/PIF.hpp
  mmio/RI.cpp
  mmio/RI.hpp
  mmio/SI.cpp
  mmio/SI.hpp
  mmio/VI.cpp
  mmio/VI.hpp
  MMIO.cpp
  MMIO.hpp
  RSP.cpp
  RSP.hpp
  rsp/decode.cpp
  rsp/instructions.cpp)

target_include_directories(core PUBLIC
  .
  ..
  ../..
  ../../frontend/imgui/debugger
  ../../../external
  ../../../external/capstone
  ../../../external/imgui/imgui
  ../../../external/imgui/imgui/debugger
  ../../../external/imgui/imgui/backends
  mmio)

target_link_libraries(core PUBLIC capstone SDL2main SDL2 fmt cpu parallel-rdp)
