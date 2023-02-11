file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

add_library(backend ${SOURCES} ${HEADERS})

if(WIN32)
  target_compile_definitions(backend PUBLIC DISABLE_GDB_STUB)
endif()