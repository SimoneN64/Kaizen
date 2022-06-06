cmake_minimum_required(VERSION 3.20)
project(natsukashii CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(src)
add_executable(natsukashii src/main.cpp)
target_include_directories(natsukashii PUBLIC src/frontend/)
target_link_libraries(natsukashii cores frontend)
