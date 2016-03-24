set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O2 -fcx-fortran-rules -fcx-limited-range -fPIC")

set(SOURCES
    "src/HelAmps_sm.cc"
    "src/Parameters_sm.cc"
    "SubProcesses/P1_Sigma_sm_gg_mupvmbmumvmxbx/cpp_pp_ttx_fullylept.cc"
    )

include_directories("include")
include_directories("SubProcesses/P1_Sigma_sm_gg_mupvmbmumvmxbx/")

add_library(me_pp_ttx_fully_leptonic STATIC ${SOURCES})
