set(SOURCES
    "src/HelAmps_sm.cc"
    "src/Parameters_sm.cc"
    "SubProcesses/P1_Sigma_sm_uux_epvemumvmx/P1_Sigma_sm_uux_epvemumvmx.cc"
    )

include_directories("include")
include_directories("SubProcesses/P1_Sigma_sm_uux_epvemumvmx/")

add_library(me_pp_WW_fully_leptonic STATIC ${SOURCES})
