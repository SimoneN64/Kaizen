file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

add_subdirectory(core)
option(CAPSTONE_ARCHITECTURE_DEFAULT OFF)
option(CAPSTONE_MIPS_SUPPORT ON)
add_subdirectory(../../external/capstone capstone)

add_library(backend ${SOURCES} ${HEADERS})
target_link_libraries(backend PRIVATE core capstone)