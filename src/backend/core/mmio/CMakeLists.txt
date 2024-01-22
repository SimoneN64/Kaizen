file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.hpp)

set(GAINPUT_SAMPLES OFF)
set(GAINPUT_TESTS OFF)
add_subdirectory(../../../../external/gainput gainput)

add_library(mmio ${SOURCES} ${HEADERS} ../../../../external/cic_nus_6105/n64_cic_nus_6105.cpp)

target_include_directories(mmio PRIVATE ../../../../external/gainput/lib/include)
target_link_libraries(mmio PRIVATE gainput)