file(GLOB SOURCES *.c)
add_library(lua STATIC ${SOURCES})

target_compile_definitions(lua PRIVATE -DLUA_USE_LINUX)
set_property(TARGET lua PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
