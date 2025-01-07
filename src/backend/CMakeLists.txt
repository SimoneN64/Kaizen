file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

add_subdirectory(core)

add_library(backend ${SOURCES} ${HEADERS})
target_link_libraries(backend PRIVATE core)