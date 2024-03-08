cmake_minimum_required (VERSION 3.8.1)

# imgui library: imgui is build by default, but you can
# provide your own version by setting QTIMGUI_BUILD_IMGUI to OFF
option(QTIMGUI_BUILD_IMGUI "Use imgui provided as qtimgui submodule" ON)
if (QTIMGUI_BUILD_IMGUI)
    add_library(imgui
      STATIC
      imgui/imconfig.h
      imgui/imgui.cpp
      imgui/imgui.h
      imgui/imgui_demo.cpp
      imgui/imgui_draw.cpp
      imgui/imgui_internal.h
      imgui/imgui_widgets.cpp
      imgui/imgui_tables.cpp
      imgui/imstb_rectpack.h
      imgui/imstb_textedit.h
      imgui/imstb_truetype.h
    )
    target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
endif(QTIMGUI_BUILD_IMGUI)