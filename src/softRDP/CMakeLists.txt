find_package(SDL2 CONFIG REQUIRED)

add_library(softRDP softrdp.hpp softrdp.cpp)
target_link_libraries(softRDP PRIVATE SDL2::SDL2)