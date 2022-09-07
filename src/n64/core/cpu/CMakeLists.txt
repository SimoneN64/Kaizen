cmake_minimum_required(VERSION 3.20)
project(cpu)

add_library(cpu
  decode.cpp
  instructions.cpp
  Registers.cpp
  Registers.hpp
  registers/Cop0.cpp
  registers/Cop0.hpp
  registers/cop0instructions.cpp
  registers/Cop1.cpp
  registers/Cop1.hpp
  registers/cop1instructions.cpp)

target_include_directories(cpu PUBLIC registers
  . ..
  ../../../../external
  ../../../../external/capstone/include
  ../../../../src
  ../../
)
