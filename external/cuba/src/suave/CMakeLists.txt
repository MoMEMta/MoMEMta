set(SOURCES Suave.c)
add_library(suave OBJECT ${SOURCES})

target_include_directories(suave PRIVATE .)
