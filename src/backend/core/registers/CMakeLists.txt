file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.cpp)

add_library(registers ${SOURCES} ${HEADERS})
