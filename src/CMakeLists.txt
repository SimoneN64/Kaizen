cmake_minimum_required(VERSION 3.20)
project(gadolinium)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

option(RAPIDJSON_BUILD_DOC "Build rapidjson documentation." OFF)
option(RAPIDJSON_BUILD_EXAMPLES "Build rapidjson examples." OFF)
option(RAPIDJSON_BUILD_TESTS "Build rapidjson perftests and unittests." OFF)

include_directories(
  .
  utils
  frontend
  frontend/imgui
  backend
  backend/core
  backend/core/mmio
  backend/core/registers
  backend/core/rsp
  ../external
  ../external/xbyak
  ../external/parallel-rdp
  ../external/parallel-rdp/parallel-rdp-standalone/parallel-rdp
  ../external/parallel-rdp/parallel-rdp-standalone/volk
  ../external/parallel-rdp/parallel-rdp-standalone/spirv-cross
  ../external/parallel-rdp/parallel-rdp-standalone/vulkan
  ../external/parallel-rdp/parallel-rdp-standalone/vulkan-headers/include
  ../external/parallel-rdp/parallel-rdp-standalone/util
  ../external/nativefiledialog-extended/src/include
  ../external/imgui/imgui
  ../external/imgui/imgui/backends
  ../external/discord-rpc/include
  ../external/gdbstub
  ${SDL2_INCLUDE_DIRS}
)

add_subdirectory(frontend)
add_subdirectory(frontend/imgui)
add_subdirectory(backend)
add_subdirectory(backend/core)
add_subdirectory(backend/core/dynarec)
add_subdirectory(backend/core/interpreter)
add_subdirectory(backend/core/mmio)
add_subdirectory(backend/core/registers)
add_subdirectory(backend/core/rsp)
add_subdirectory(../external/discord-rpc discord-rpc)
add_subdirectory(../external/imgui imgui)
add_subdirectory(../external/nativefiledialog-extended nfd)
add_subdirectory(../external/parallel-rdp parallel-rdp)

add_executable(gadolinium main.cpp)

file(COPY ${PROJECT_SOURCE_DIR}/../resources/ DESTINATION ${PROJECT_BINARY_DIR}/resources/)
file(REMOVE
  ${PROJECT_BINARY_DIR}/resources/mario.png
  ${PROJECT_BINARY_DIR}/resources/shader.frag
  ${PROJECT_BINARY_DIR}/resources/shader.vert)

target_link_libraries(gadolinium PUBLIC frontend frontend-imgui
  discord-rpc imgui nfd parallel-rdp backend fmt::fmt nlohmann_json::nlohmann_json core dynarec registers interpreter mmio rsp SDL2::SDL2main SDL2::SDL2)

if(WIN32)
  target_compile_definitions(gadolinium PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
  if(MSVC)
    target_compile_options(parallel-rdp PUBLIC /EHa)
  endif()
endif()