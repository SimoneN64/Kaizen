file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.hpp)

find_package(SDL2 REQUIRED)

add_library(mmio ${SOURCES} ${HEADERS} ../../../../external/cic_nus_6105/n64_cic_nus_6105.cpp)

target_link_libraries(mmio PRIVATE SDL2::SDL2)