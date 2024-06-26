cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

include(CheckCCompilerFlag)

check_c_compiler_flag(-msse4.1 HAS_SSE4_1)

if (HAS_SSE4_1)
  add_compile_definitions(SIMD_SUPPORT)
  add_compile_options(-msse3 -msse4.1)
endif ()

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  add_compile_definitions(VULKAN_DEBUG)
  #add_compile_options(-fsanitize=address -fsanitize=undefined)
  #add_link_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-Wl,--export-dynamic)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets)

if (Qt6_FOUND)
  add_subdirectory(src/frontend)
endif()