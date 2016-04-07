set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O2 -fcx-fortran-rules -fcx-limited-range -fPIC")

set(SOURCES
    "dummy_me.cc"
    )

add_library(me_dummy SHARED ${SOURCES})
