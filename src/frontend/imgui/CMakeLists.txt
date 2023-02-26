file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

if(WIN32)
  add_compile_definitions(SDL_MAIN_HANDLED)
endif()

add_library(frontend-imgui ${SOURCES} ${HEADERS})