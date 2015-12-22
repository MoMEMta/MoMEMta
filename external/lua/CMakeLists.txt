add_definitions(-DLUA_USE_LINUX)

file(GLOB SOURCES *.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fomit-frame-pointer -Wall -fPIC")

add_library(lua STATIC ${SOURCES})
