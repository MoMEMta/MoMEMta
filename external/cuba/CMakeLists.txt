# Always build with PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DHAVE_CONFIG_H -DREAL_SIZE=8 -DNOUNDERSCORE -DLONGLONGINT)
include_directories(. src/common)

set(SOURCES src/common/Fork.c src/common/Global.c src/common/Data.c)
add_library(cuba STATIC ${SOURCES})

add_subdirectory(src/vegas)
add_subdirectory(src/suave)
add_subdirectory(src/divonne)
add_subdirectory(src/cuhre)

target_sources(cuba INTERFACE $<TARGET_OBJECTS:vegas> $<TARGET_OBJECTS:suave> $<TARGET_OBJECTS:divonne> $<TARGET_OBJECTS:cuhre>)
target_include_directories(cuba PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
