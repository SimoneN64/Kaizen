file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

add_library(frontend-imgui ${SOURCES} ${HEADERS})