#cmake_minimum_required(VERSION 2.6)
#
## CMAKE_CURRENT_LIST_DIR cmake 2.6 compatibility
#if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
#    get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
#endif(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
#
#file(GLOB_RECURSE SOURCES *.cc)
#add_library(ME_example_WW_llvv STATIC ${SOURCES})
#
#include_directories("${CMAKE_CURRENT_LIST_DIR}/include")
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fcx-fortran-rules -fcx-limited-range")

set(SOURCES
    "src/HelAmps_sm.cc"
    "src/Parameters_sm.cc"
    "SubProcesses/P1_Sigma_sm_uux_epvemumvmx/P1_Sigma_sm_uux_epvemumvmx.cc"
    )

include_directories("include")
include_directories("SubProcesses/P1_Sigma_sm_uux_epvemumvmx")

add_library(me_example_WW_llvv STATIC ${SOURCES})
