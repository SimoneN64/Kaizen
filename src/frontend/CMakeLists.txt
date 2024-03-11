cmake_minimum_required(VERSION 3.20)
project(kaizen-qt)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

include_directories(
  .
  ../
  ../utils
  ../backend
  ../backend/core
  ../backend/core/mmio
  ../backend/core/mmio/PIF
  ../backend/core/registers
  ../backend/core/rsp
  ../../external/
  ../../external/enet/include
  ../../external/xbyak
  ../../external/mio/include
  ../../external/fmt/include
  ../../external/json/include
  ../../external/parallel-rdp
  ../../external/parallel-rdp/parallel-rdp-standalone/parallel-rdp
  ../../external/parallel-rdp/parallel-rdp-standalone/volk
  ../../external/parallel-rdp/parallel-rdp-standalone/spirv-cross
  ../../external/parallel-rdp/parallel-rdp-standalone/vulkan
  ../../external/parallel-rdp/parallel-rdp-standalone/vulkan-headers/include
  ../../external/parallel-rdp/parallel-rdp-standalone/util
  ../../external/nativefiledialog-extended/src/include
  ../../external/imgui/imgui
  ../../external/imgui/imgui/backends
  ../../external/unarr
)

option(RAPIDJSON_BUILD_DOC "Build rapidjson documentation." OFF)
option(RAPIDJSON_BUILD_EXAMPLES "Build rapidjson examples." OFF)
option(RAPIDJSON_BUILD_TESTS "Build rapidjson perftests and unittests." OFF)

add_subdirectory(../../external/json json)
add_subdirectory(../../external/fmt fmt)
add_subdirectory(../../external/mio mio)
add_subdirectory(../backend backend)
add_subdirectory(../../external/nativefiledialog-extended nfd)
add_subdirectory(../../external/parallel-rdp parallel-rdp)
add_subdirectory(../../external/unarr unarr)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(kaizen-qt
  main.cpp
  KaizenQt.hpp
  KaizenQt.cpp
  RenderWidget.cpp
  RenderWidget.hpp
  EmuThread.hpp
  EmuThread.cpp
  mainwindow.ui
  MainWindow.hpp
  MainWindow.cpp
  SettingsWindow.hpp
  SettingsWindow.cpp
  CPUSettings.hpp
  CPUSettings.cpp
  JSONUtils.hpp
  AudioSettings.hpp
  AudioSettings.cpp
  InputSettings.hpp
  InputSettings.cpp
  NetplayWindow.hpp
  NetplayWindow.cpp)

target_link_libraries(kaizen-qt PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets fmt mio nlohmann_json nfd parallel-rdp backend)

file(COPY ../../resources/ DESTINATION ${PROJECT_BINARY_DIR}/resources/)
file(REMOVE
  ${PROJECT_BINARY_DIR}/resources/mario.png
  ${PROJECT_BINARY_DIR}/resources/shader.frag
  ${PROJECT_BINARY_DIR}/resources/shader.vert)