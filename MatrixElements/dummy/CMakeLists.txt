set(SOURCES
    "dummy_me.cc"
    )

add_library(me_dummy SHARED ${SOURCES})
target_link_libraries(me_dummy momemta)
