cmake_minimum_required(VERSION 3.20)
project(kaizen)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)
find_package(mio REQUIRED)
find_package(nlohmann_json REQUIRED)

option(RAPIDJSON_BUILD_DOC "Build rapidjson documentation." OFF)
option(RAPIDJSON_BUILD_EXAMPLES "Build rapidjson examples." OFF)
option(RAPIDJSON_BUILD_TESTS "Build rapidjson perftests and unittests." OFF)

include_directories(
  src
  src/utils
  src/frontend
  src/frontend/imgui
  src/backend
  src/backend/core
  src/backend/core/mmio
  src/backend/core/registers
  src/backend/core/rsp
  external
  external/xbyak
  external/parallel-rdp
  external/parallel-rdp/parallel-rdp-standalone/parallel-rdp
  external/parallel-rdp/parallel-rdp-standalone/volk
  external/parallel-rdp/parallel-rdp-standalone/spirv-cross
  external/parallel-rdp/parallel-rdp-standalone/vulkan
  external/parallel-rdp/parallel-rdp-standalone/vulkan-headers/include
  external/parallel-rdp/parallel-rdp-standalone/util
  external/nativefiledialog-extended/src/include
  external/imgui/imgui
  external/imgui/imgui/backends
  external/discord-rpc/include
  external/unarr
  ${SDL2_INCLUDE_DIRS}
)

add_compile_definitions(SIMD_SUPPORT)
if(WIN32)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()
add_compile_options(-mssse3 -msse4.1)
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  #add_compile_options(-fsanitize=address -fsanitize=undefined)
  #add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_subdirectory(src/frontend)
add_subdirectory(src/frontend/imgui)
add_subdirectory(src/backend)
add_subdirectory(src/backend/netplay)
add_subdirectory(src/backend/core)
add_subdirectory(src/backend/core/interpreter)
add_subdirectory(src/backend/core/JIT)
add_subdirectory(src/backend/core/mem)
add_subdirectory(src/backend/core/mmio)
add_subdirectory(src/backend/core/registers)
add_subdirectory(src/backend/core/rsp)
add_subdirectory(external/discord-rpc)
add_subdirectory(external/imgui)
add_subdirectory(external/nativefiledialog-extended)
add_subdirectory(external/parallel-rdp)
add_subdirectory(external/unarr)

add_executable(kaizen main.cpp)

if(MSVC)
  target_compile_options(parallel-rdp PUBLIC /EHa)
endif()

file(COPY ${PROJECT_SOURCE_DIR}/resources/ DESTINATION ${PROJECT_BINARY_DIR}/resources/)
file(REMOVE
  ${PROJECT_BINARY_DIR}/resources/mario.png
  ${PROJECT_BINARY_DIR}/resources/shader.frag
  ${PROJECT_BINARY_DIR}/resources/shader.vert)

target_link_libraries(kaizen PUBLIC frontend frontend-imgui
  discord-rpc imgui nfd parallel-rdp backend fmt::fmt mio::mio nlohmann_json::nlohmann_json core registers jit interpreter mem unarr mmio rsp SDL2::SDL2main SDL2::SDL2)
