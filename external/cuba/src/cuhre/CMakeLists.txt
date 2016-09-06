set(SOURCES Cuhre.c)
add_library(cuhre OBJECT ${SOURCES})

target_include_directories(cuhre PRIVATE .)
