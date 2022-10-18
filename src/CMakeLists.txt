cmake_minimum_required(VERSION 3.20)
project(natsukashii)

set(BUILD_SHARED_LIBS OFF)

file(GLOB_RECURSE CORE_SOURCES n64/*.cpp)
file(GLOB_RECURSE CORE_HEADERS n64/*.hpp)
file(GLOB_RECURSE FRONTEND_SOURCES frontend/*.cpp)
file(GLOB_RECURSE FRONTEND_HEADERS frontend/*.hpp)

find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(../external/capstone capstone)
add_subdirectory(../external/parallel-rdp prdp)
add_subdirectory(../external/imgui imgui)
add_subdirectory(../external/nativefiledialog-extended nfd)

add_executable(natsukashii
  ${CORE_SOURCES}
  ${CORE_HEADERS}
  ${FRONTEND_SOURCES}
  ${FRONTEND_HEADERS}
  main.cpp
  Scheduler.cpp
  Scheduler.hpp
  common.hpp
  util.hpp
)

target_include_directories(natsukashii PRIVATE
  .
  n64
  n64/core
  n64/core/cpu/
  n64/core/cpu/registers
  n64/core/mmio
  n64/core/rsp
  frontend
  frontend/imgui
  ../external
  ../external/imgui/imgui
  ../external/parallel-rdp/
  ../external/nativefiledialog-extended/src/include
  ../external/capstone/include
)

if(WIN32)
  target_compile_options(natsukashii PRIVATE -mwindows)
  set(LIBRARIES -static z stdc++ user32 gdi32 winmm Imm32 ole32 oleaut32 shell32 setupapi version uuid)
else()
  set(LIBRARIES)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Release)
  target_compile_options(natsukashii PRIVATE -Ofast)
elseif(${CMAKE_BUILD_TYPE} MATCHES Debug)
  target_compile_options(natsukashii PRIVATE -g)
endif()

file(COPY ${PROJECT_SOURCE_DIR}/../resources/ DESTINATION ${PROJECT_BINARY_DIR}/resources/)
file(REMOVE
  ${PROJECT_BINARY_DIR}/resources/mario.png
  ${PROJECT_BINARY_DIR}/resources/shader.frag
  ${PROJECT_BINARY_DIR}/resources/shader.vert)

target_link_libraries(natsukashii PRIVATE ${LIBRARIES} SDL2 capstone-static nfd parallel-rdp imgui fmt::fmt nlohmann_json::nlohmann_json)