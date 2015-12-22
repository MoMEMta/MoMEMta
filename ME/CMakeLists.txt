cmake_minimum_required(VERSION 2.6)

# CMAKE_CURRENT_LIST_DIR cmake 2.6 compatibility
if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
    get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)

file(GLOB_RECURSE SOURCES *.cc)
add_library(ME_cpp_pp_ttx_fullylept SHARED ${SOURCES})

include_directories("${CMAKE_CURRENT_LIST_DIR}/src")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fcx-fortran-rules -fcx-limited-range")
