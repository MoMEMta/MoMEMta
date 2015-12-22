add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DREAL_SIZE=8)
add_definitions(-DNOUNDERSCORE)

#add_definitions(-DLONGLONGINT)

include_directories(.)
include_directories(src/common)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fomit-frame-pointer -ffast-math -Wall -fPIC")

add_subdirectory(src/vegas)
add_subdirectory(src/suave)
add_subdirectory(src/divonne)
add_subdirectory(src/cuhre)

set(SOURCES src/common/Fork.c src/common/Global.c src/common/Data.c)

add_library(cuba STATIC ${SOURCES} $<TARGET_OBJECTS:vegas> $<TARGET_OBJECTS:suave> $<TARGET_OBJECTS:divonne> $<TARGET_OBJECTS:cuhre>)
