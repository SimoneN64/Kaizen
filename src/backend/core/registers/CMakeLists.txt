file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.cpp)

add_library(registers ${SOURCES} ${HEADERS})