set(SOURCES
    "src/HelAmps_sm.cc"
    "src/Parameters_sm.cc"
    "SubProcesses/P1_Sigma_sm_uux_epembbx/P1_Sigma_sm_uux_epembbx.cc"
    )

add_library(me_pp_zh_z_ee_h_bb OBJECT ${SOURCES})

target_include_directories(me_pp_zh_z_ee_h_bb PRIVATE "include")
target_include_directories(me_pp_zh_z_ee_h_bb PRIVATE "SubProcesses/P1_Sigma_sm_uux_epembbx/")
