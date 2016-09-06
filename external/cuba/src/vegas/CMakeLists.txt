set(SOURCES Vegas.c)
add_library(vegas OBJECT ${SOURCES})

target_include_directories(vegas PRIVATE .)
