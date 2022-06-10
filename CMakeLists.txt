cmake_minimum_required(VERSION 3.20)
project(natsukashii CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_subdirectory(src/core)

option(BUILD_SDL "Build the SDL frontend" ON)
option(BUILD_QT "Build the QT frontend" ON)

if(${BUILD_QT})
  add_subdirectory(src/frontend/qt ${CMAKE_CURRENT_BINARY_DIR}/bin/qt/)
endif()

if(${BUILD_SDL})
  add_subdirectory(src/frontend/sdl ${CMAKE_CURRENT_BINARY_DIR}/bin/sdl/)
endif()
