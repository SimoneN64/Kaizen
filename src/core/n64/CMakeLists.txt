cmake_minimum_required(VERSION 3.20)
project(n64 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(mio REQUIRED)
find_package(fmt REQUIRED)
#find_package(toml11 REQUIRED)

add_library(n64 Core.hpp Core.cpp Cpu.hpp Cpu.cpp Mem.cpp Mem.hpp memory_regions.hpp)
target_include_directories(n64 PRIVATE . ..)
target_link_libraries(n64 PUBLIC fmt::fmt)
