cmake_minimum_required(VERSION 3.20)
project(n64 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  mio
  GIT_REPOSITORY https://github.com/CocoSimone/mio
  GIT_TAG master
)
FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11
  GIT_TAG dcfe39a783a94e8d52c885e5883a6fbb21529019
)

FetchContent_MakeAvailable(mio toml11)
find_package(fmt REQUIRED)

add_subdirectory(core)

set(PARALLELRDP_INCLUDES
  ../../frontend
  ../../../external/parallel-rdp-standalone/vulkan
  ../../../external/parallel-rdp-standalone/util
  ../../../external/parallel-rdp-standalone/parallel-rdp
  ../../../external/parallel-rdp-standalone/volk
  ../../../external/parallel-rdp-standalone/vulkan-headers)

add_library(n64
  Core.hpp
  Core.cpp
  memory_regions.hpp
  ../BaseCore.cpp
  ../BaseCore.hpp)
target_include_directories(n64 PRIVATE . .. ${PARALLELRDP_INCLUDES})
target_include_directories(n64 PUBLIC ${mio_SOURCE_DIR}/include ${toml11_SOURCE_DIR}/include)
target_link_libraries(n64 PRIVATE mio::mio toml11::toml11 fmt::fmt n64-core)
