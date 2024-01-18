cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(SIMD_SUPPORT)
if(WIN32)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

include(CheckCCompilerFlag)

check_c_compiler_flag(-mssse3 HAS_SSSE3)
check_c_compiler_flag(-msse4.1 HAS_SSE4_1)

if (HAS_SSSE3 AND HAS_SSE4_1)
  add_compile_options(-mssse3 -msse4.1)
endif ()

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  add_compile_definitions(VULKAN_DEBUG)
  #add_compile_options(-fsanitize=address -fsanitize=undefined)
  #add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets)

if (Qt6_FOUND)
  add_subdirectory(src/frontend)
endif()