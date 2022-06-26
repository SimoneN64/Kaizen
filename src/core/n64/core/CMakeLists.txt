cmake_minimum_required(VERSION 3.20)
project(n64-core)

add_subdirectory(cpu)

add_library(n64-core
  Cpu.hpp
  Cpu.cpp
  Mem.cpp
  Mem.hpp RDP.cpp RDP.hpp)

target_include_directories(n64-core PRIVATE . .. ../../)
target_link_libraries(n64-core PUBLIC n64-cpu)
