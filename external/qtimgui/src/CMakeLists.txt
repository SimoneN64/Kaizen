cmake_minimum_required (VERSION 3.8.1)
project(qtimgui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# We're using qt versionless targets so our minimal Qt version is 5.15, we support Qt6 as well
# See https://www.qt.io/blog/versionless-cmake-targets-qt-5.15
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGLWidgets REQUIRED)

message(STATUS  Qt ${QT_DEFAULT_MAJOR_VERSION})

set(
    qt_imgui_sources
    key_mappings.h
    ImGuiRenderer.cpp
    QtImGui.cpp
)

# qt_imgui_widget: library with a qt renderer for classic Qt Widget applications
add_library(qt_imgui_widgets STATIC ${qt_imgui_sources})
target_include_directories(qt_imgui_widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
    qt_imgui_widgets
    PUBLIC
    imgui
    Qt::Widgets
    # We use the new Qt Rendering Hardware Interface (RHI) when buzilding with Qt and thgus we need openglwidgets qt module
    # see https://www.qt.io/blog/qt-5.15-released
    $<$<BOOL:${Qt6_FOUND}>:Qt::OpenGLWidgets>
    )
if (ANDROID)
    target_link_libraries(qt_imgui_widgets PUBLIC log dl GLESv2 z)
endif()
target_compile_definitions(qt_imgui_widgets PUBLIC QT_WIDGETS_LIB)
