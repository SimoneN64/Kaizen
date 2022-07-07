cmake_minimum_required(VERSION 3.20)
project(parallel-rdp)

file(GLOB_RECURSE parallel-rdp-cpp parallel-rdp-standalone/parallel-rdp/*.cpp)

add_library(parallel-rdp
  ${parallel-rdp-cpp}
  vulkan/buffer.cpp
  vulkan/buffer_pool.cpp
  vulkan/command_buffer.cpp
  vulkan/command_pool.cpp
  vulkan/context.cpp
  vulkan/cookie.cpp
  vulkan/descriptor_set.cpp
  vulkan/device.cpp
  vulkan/event_manager.cpp
  vulkan/fence.cpp
  vulkan/fence_manager.cpp
  vulkan/image.cpp
  vulkan/memory_allocator.cpp
  vulkan/pipeline_event.cpp
  vulkan/query_pool.cpp
  vulkan/render_pass.cpp
  vulkan/sampler.cpp
  vulkan/semaphore.cpp
  vulkan/semaphore_manager.cpp
  vulkan/shader.cpp
  vulkan/texture_format.cpp
  vulkan/wsi.cpp
  vulkan/wsi.hpp
  vulkan/wsi_timing.cpp
  vulkan/wsi_timing.hpp
  util/aligned_alloc.cpp
  util/timer.cpp
  util/timeline_trace_file.cpp
  util/timeline_trace_file.hpp
  util/thread_name.cpp
  util/thread_name.hpp
  util/logging.cpp
  util/logging.hpp
  util/thread_id.cpp
  util/thread_id.hpp
  # C
  volk/volk.c
)

target_compile_definitions(parallel-rdp PUBLIC GRANITE_VULKAN_MT)

target_include_directories(parallel-rdp PUBLIC
  parallel-rdp
  volk
  parallel-rdp-standalone/spirv-cross
  vulkan
  vulkan-headers/include
  util
  ../src/core/n64
  )

if(WIN32)
  target_compile_definitions(parallel-rdp PUBLIC VK_USE_PLATFORM_WIN32_KHR)
else()
  target_link_libraries(parallel-rdp dl)
endif()