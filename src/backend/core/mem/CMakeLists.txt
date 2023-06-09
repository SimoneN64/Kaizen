file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

add_library(mem ${SOURCES} ${HEADERS})