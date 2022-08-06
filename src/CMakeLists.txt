cmake_minimum_required(VERSION 3.20)
project(natsukashii)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_subdirectory(n64)
add_subdirectory(frontend)

add_executable(natsukashii main.cpp)

target_link_libraries(natsukashii PUBLIC frontend n64)
target_include_directories(natsukashii PUBLIC . ../external)
