add_library(imgui
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_internal.h
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui.cpp
  imgui/imgui.h
  imgui/imconfig.h
  imgui/backends/imgui_impl_sdl.h
  imgui/backends/imgui_impl_sdl.cpp
  imgui/backends/imgui_impl_vulkan.h
  imgui/backends/imgui_impl_vulkan.cpp)

if(WIN32)
  target_compile_definitions(imgui PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
  if(MSVC)
    target_compile_options(imgui PUBLIC /EHa)
  endif()
endif()

target_include_directories(imgui PUBLIC ../parallel-rdp/parallel-rdp-standalone/volk
  ../parallel-rdp/parallel-rdp-standalone/vulkan-headers/include)