cmake_minimum_required(VERSION 3.20)
project(gb CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(mio REQUIRED)
find_package(fmt REQUIRED)

add_library(gb Core.hpp Core.cpp Cpu.hpp Cpu.cpp Ppu.hpp Ppu.cpp Mem.cpp Mem.hpp mbc.cpp mbc.hpp memory_regions.hpp)
target_include_directories(gb PUBLIC . ..)
target_link_libraries(gb PUBLIC mio::mio fmt::fmt)
