set(SOURCES Divonne.c)
add_library(divonne OBJECT ${SOURCES})

target_include_directories(divonne PRIVATE .)
