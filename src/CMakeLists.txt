cmake_minimum_required(VERSION 3.20)
project(gadolinium)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE CORE_SOURCES backend/*.cpp)
file(GLOB_RECURSE CORE_HEADERS backend/*.hpp)
file(GLOB_RECURSE FRONTEND_SOURCES frontend/*.cpp)
file(GLOB_RECURSE FRONTEND_HEADERS frontend/*.hpp)

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(nlohmann_json REQUIRED)

option(RAPIDJSON_BUILD_DOC "Build rapidjson documentation." OFF)
option(RAPIDJSON_BUILD_EXAMPLES "Build rapidjson examples." OFF)
option(RAPIDJSON_BUILD_TESTS "Build rapidjson perftests and unittests." OFF)

add_subdirectory(../external/capstone capstone)
add_subdirectory(../external/parallel-rdp prdp)
add_subdirectory(../external/imgui imgui)
add_subdirectory(../external/nativefiledialog-extended nfd)
add_subdirectory(../external/discord-rpc discord-rpc)

add_executable(gadolinium
  ${CORE_SOURCES}
  ${CORE_HEADERS}
  ${FRONTEND_SOURCES}
  ${FRONTEND_HEADERS}
  main.cpp
  common.hpp
  backend/Scheduler.cpp
  backend/Scheduler.hpp)

target_include_directories(gadolinium PRIVATE
  .
  backend
  backend/core
  backend/core/interpreter/
  backend/core/registers
  backend/core/mmio
  backend/core/rsp
  frontend
  frontend/imgui
  utils
  ../external
  ../external/xbyak
  ../external/imgui/imgui
  ../external/parallel-rdp/
  ../external/nativefiledialog-extended/src/include
  ../external/capstone/include
  ../external/discord-rpc/include
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

if(${CMAKE_BUILD_TYPE} MATCHES Release)
  set_property(TARGET gadolinium PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(WIN32)
    add_compile_options(/O2)
  else()
    add_compile_options(-O3)
  endif()
else()
  if(WIN32)
    add_compile_options(/Od)
  else()
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    add_compile_options(-g)
  endif()
endif()

target_link_libraries(gadolinium PRIVATE discord-rpc SDL2::SDL2main SDL2::SDL2 capstone-static nfd parallel-rdp fmt::fmt imgui nlohmann_json::nlohmann_json)