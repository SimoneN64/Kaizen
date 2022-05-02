cmake_minimum_required(VERSION 3.20)
add_subdirectory(core)

set(FRONTEND "qt" CACHE INTERNAL "")

if(${FRONTEND} MATCHES "qt")
  add_subdirectory(frontend/qt)
elseif(${FRONTEND} MATCHES "sdl")
  add_subdirectory(frontend/sdl)
else()
  message("Invalid frontend specified, falling back to Qt.")
  add_subdirectory(frontend/qt)
endif()
