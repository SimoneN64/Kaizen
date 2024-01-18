file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

add_subdirectory(interpreter)
add_subdirectory(mem)
add_subdirectory(mmio)
add_subdirectory(registers)
add_subdirectory(rsp)

add_library(core ${SOURCES} ${HEADERS})
target_link_libraries(core PRIVATE interpreter mem mmio unarr registers rsp)