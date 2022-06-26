cmake_minimum_required(VERSION 3.20)
project(n64-cpu)

add_library(n64-cpu
  Registers.cpp
  Registers.hpp
  registers/Cop0.cpp
  registers/Cop0.hpp)

target_include_directories(n64-cpu PRIVATE . .. ../../ ../../../)
target_include_directories(n64-cpu PUBLIC registers ../../../../../external)
