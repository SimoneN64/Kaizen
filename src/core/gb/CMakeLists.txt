cmake_minimum_required(VERSION 3.20)
project(gb CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(mio REQUIRED)
find_package(fmt REQUIRED)
find_package(toml11 REQUIRED)

add_library(gb Core.hpp Core.cpp Cpu.hpp Cpu.cpp Ppu.hpp Ppu.cpp Mem.cpp Mem.hpp mbc.cpp mbc.hpp memory_regions.hpp)
target_include_directories(gb PRIVATE . ..)
target_link_libraries(gb PUBLIC toml11::toml11 mio::mio fmt::fmt)
