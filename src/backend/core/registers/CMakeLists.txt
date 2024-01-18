file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.cpp)

add_library(registers ${SOURCES} ${HEADERS})
target_link_libraries(registers PRIVATE interpreter)
