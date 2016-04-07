add_definitions(-DLUA_USE_LINUX)

file(GLOB SOURCES *.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(lua STATIC ${SOURCES})
