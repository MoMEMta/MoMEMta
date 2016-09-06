set(SOURCES
    "src/HelAmps_sm.cc"
    "src/Parameters_sm.cc"
    "SubProcesses/P1_Sigma_sm_gg_mupvmbmumvmxbx/cpp_pp_ttx_fullylept.cc"
    )

add_library(me_pp_ttx_fully_leptonic OBJECT ${SOURCES})

target_include_directories(me_pp_ttx_fully_leptonic PRIVATE "include")
target_include_directories(me_pp_ttx_fully_leptonic PRIVATE "SubProcesses/P1_Sigma_sm_gg_mupvmbmumvmxbx/")
