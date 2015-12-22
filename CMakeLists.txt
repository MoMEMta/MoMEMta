cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

# CMAKE_CURRENT_LIST_DIR cmake 2.6 compatibility
if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
    get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

# Find dependencies
find_package(ROOT REQUIRED)
find_package(LHAPDF REQUIRED)
find_package(Lua)

# Include external
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/external")
add_subdirectory(external)
add_subdirectory(ME)

if (NOT LUA_FOUND)
    set(LUA_LIBRARIES lua)
    set(LUA_INCLUDE_DIR "external/lua")
endif()

set(MEMPP_SOURCES
    "modules/BlockD.cc"
    "modules/Boost.cc"
    "modules/Flatter.cc"
    "modules/GaussianTransferFunction.cc"
    "modules/MatrixElement.cc"
    "src/ConfigurationReader.cc"
    "src/ConfigurationSet.cc"
    "src/InputTag.cc"
    "src/LibraryManager.cc"
    "src/logging.cc"
    "src/MEMpp.cc"
    "src/ModuleFactory.cc"
    "src/Pool.cc"
    "src/SharedLibrary.cc"
    "src/Utils.cc"
    "src/lua/utils.cc"
    )

add_library(mempp SHARED ${MEMPP_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O2 -fcx-fortran-rules -fcx-limited-range")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(ME/src)
include_directories(ME/SubProcesses/P1_Sigma_sm_gg_mupvmbmumvmxbx)
include_directories(${EXTERNAL_DIR})
include_directories(${EXTERNAL_DIR}/cuba)
include_directories(${EXTERNAL_DIR}/spdlog)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${LHAPDF_INCLUDE_DIRECTORIES})
include_directories(${ROOT_INCLUDE_DIR})

target_link_libraries(mempp dl)
target_link_libraries(mempp cuba)
target_link_libraries(mempp ${LUA_LIBRARIES})
target_link_libraries(mempp pthread)
target_link_libraries(mempp ME_cpp_pp_ttx_fullylept)
target_link_libraries(mempp ${LHAPDF_LIBRARIES})
target_link_libraries(mempp ${ROOT_LIBRARIES})

find_library(ROOT_GENVECTOR_LIBRARY GenVector HINTS ${ROOT_LIBRARY_DIR})
target_link_libraries(mempp ${ROOT_GENVECTOR_LIBRARY})

add_library(empty_module SHARED "modules/EmptyModule.cc")

add_executable(main "src/main.cc")
target_link_libraries(main mempp)
set_target_properties(main PROPERTIES OUTPUT_NAME "mempp.exe")
