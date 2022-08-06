cmake_minimum_required(VERSION 3.20)
project(cpu)

add_library(cpu
  Registers.cpp
  Registers.hpp
  registers/Cop0.cpp
  registers/Cop0.hpp
  decode.cpp
  registers/cop0instructions.cpp
  registers/Cop1.cpp
  registers/Cop1.hpp
  registers/cop1instructions.cpp
  instructions.cpp)

target_include_directories(cpu PUBLIC registers . .. ../../
  ../../../../../external ../../../)
