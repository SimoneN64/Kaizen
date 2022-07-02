cmake_minimum_required(VERSION 3.20)
project(n64-core)

add_subdirectory(cpu)

add_subdirectory(../../../../external temp)

add_library(n64-core
  Cpu.hpp
  Cpu.cpp
  Mem.cpp
  Mem.hpp
  RDP.cpp
  RDP.hpp
  mmio/VI.cpp
  mmio/VI.hpp
  mmio/Interrupt.hpp
  mmio/MI.cpp
  mmio/MI.hpp
  mmio/Interrupt.cpp
  MMIO.cpp
  MMIO.hpp
  RSP.cpp
  RSP.hpp
  rsp/decode.cpp
  rsp/instructions.cpp)

target_include_directories(n64-core PRIVATE . .. ../../ mmio)
target_link_libraries(n64-core PUBLIC n64-cpu parallel-rdp)
