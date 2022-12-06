cmake_minimum_required(VERSION 3.20)
project(gadolinium)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE CORE_SOURCES n64/*.cpp)
file(GLOB_RECURSE CORE_HEADERS n64/*.hpp)
file(GLOB_RECURSE FRONTEND_SOURCES frontend/*.cpp)
file(GLOB_RECURSE FRONTEND_HEADERS frontend/*.hpp)

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(../external/capstone capstone)
add_subdirectory(../external/parallel-rdp prdp)
add_subdirectory(../external/imgui imgui)
add_subdirectory(../external/nativefiledialog-extended nfd)

add_executable(gadolinium
  ${CORE_SOURCES}
  ${CORE_HEADERS}
  ${FRONTEND_SOURCES}
  ${FRONTEND_HEADERS}
  main.cpp
  common.hpp
  util.hpp)

target_include_directories(gadolinium PRIVATE
  .
  n64
  n64/core
  n64/core/interpreter/
  n64/core/registers
  n64/core/mmio
  n64/core/rsp
  frontend
  frontend/imgui
  ../external
  ../external/xbyak
  ../external/imgui/imgui
  ../external/parallel-rdp/
  ../external/nativefiledialog-extended/src/include
  ../external/capstone/include
  ${SDL2_INCLUDE_DIR}
)

file(COPY ${PROJECT_SOURCE_DIR}/../resources/ DESTINATION ${PROJECT_BINARY_DIR}/resources/)
file(REMOVE
  ${PROJECT_BINARY_DIR}/resources/mario.png
  ${PROJECT_BINARY_DIR}/resources/shader.frag
  ${PROJECT_BINARY_DIR}/resources/shader.vert)

if(WIN32)
  target_compile_definitions(gadolinium PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
  target_compile_options(gadolinium PUBLIC /EHa)
endif()

target_link_libraries(gadolinium PRIVATE SDL2::SDL2main SDL2::SDL2 capstone-static nfd parallel-rdp fmt::fmt imgui nlohmann_json::nlohmann_json)