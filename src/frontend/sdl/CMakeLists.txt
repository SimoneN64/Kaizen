cmake_minimum_required(VERSION 3.20)
project(natsukashii-sdl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(SDL2 REQUIRED)

add_executable(natsukashii-sdl Frontend.cpp Frontend.hpp main.cpp ../ParallelRDPWrapper.cpp ../ParallelRDPWrapper.hpp)

include(FetchContent)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG 40a3888f151256bba95c45860af381b2b73752e8
)
FetchContent_MakeAvailable(argparse)

target_include_directories(natsukashii-sdl PRIVATE . ../../core)
target_link_libraries(natsukashii-sdl PRIVATE cores argparse::argparse SDL2)
