cmake_minimum_required(VERSION 3.20)
project(natsukashii)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

#add_compile_definitions(VULKAN_DEBUG)
file(COPY ${CMAKE_SOURCE_DIR}/../resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(n64)
add_subdirectory(frontend)

find_package(fmt REQUIRED)

add_executable(natsukashii main.cpp)

target_link_libraries(natsukashii PUBLIC frontend n64 fmt)
target_include_directories(natsukashii PUBLIC . ../external)
target_compile_options(natsukashii PUBLIC -Wpedantic -Wimplicit-fallthrough -Wextra -Wall)
