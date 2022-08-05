cmake_minimum_required(VERSION 3.20)
project(n64-core)

add_subdirectory(cpu)

add_subdirectory(../../../../external/parallel-rdp temp)

add_library(n64-core
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

target_include_directories(n64-core PUBLIC
  .
  ..
  ../..
  ../../../../external
  ../../../external/imgui/imgui
  ../../../external/imgui/imgui/backends
  mmio)

target_link_libraries(n64-core PUBLIC n64-cpu parallel-rdp)
