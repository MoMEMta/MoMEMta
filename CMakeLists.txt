cmake_minimum_required(VERSION 2.8.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

# Options
option(USE_BUILTIN_LUA "Force usage of built-in lua interpreter, even if a system-wide version if available" OFF)

# Find dependencies
find_package(ROOT REQUIRED)
find_package(LHAPDF 6.0 REQUIRED)

if (NOT USE_BUILTIN_LUA)
    find_package(Lua QUIET)
endif()

# Include external
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/external")
add_subdirectory(external)
add_subdirectory(ME)

if (NOT LUA_FOUND)
    if (USE_BUILTIN_LUA)
        message(STATUS "Using built-in lua interpreter")
    else()
        message(STATUS "No system-wide Lua found. Using built-in distribution")
    endif()
    set(LUA_LIBRARIES lua)
    set(LUA_INCLUDE_DIR "external/lua")
endif()


set(MOMEMTA_SOURCES
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
    "src/MoMEMta.cc"
    "src/ModuleFactory.cc"
    "src/Pool.cc"
    "src/SharedLibrary.cc"
    "src/Utils.cc"
    "src/lua/utils.cc"
    )

add_library(momemta SHARED ${MOMEMTA_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O2 -fcx-fortran-rules -fcx-limited-range")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(ME/src)
include_directories(ME/SubProcesses/P1_Sigma_sm_gg_mupvmbmumvmxbx)
include_directories(${EXTERNAL_DIR})
include_directories(${EXTERNAL_DIR}/cuba)
include_directories(${EXTERNAL_DIR}/spdlog)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${LHAPDF_INCLUDE_DIRS})
include_directories(${ROOT_INCLUDE_DIR})

target_link_libraries(momemta dl)
target_link_libraries(momemta cuba)
target_link_libraries(momemta ${LUA_LIBRARIES})
target_link_libraries(momemta pthread)
target_link_libraries(momemta ME_cpp_pp_ttx_fullylept)
target_link_libraries(momemta ${LHAPDF_LIBRARIES})
target_link_libraries(momemta ${ROOT_LIBRARIES})

find_library(ROOT_GENVECTOR_LIBRARY GenVector HINTS ${ROOT_LIBRARY_DIR})
target_link_libraries(momemta ${ROOT_GENVECTOR_LIBRARY})

add_library(empty_module SHARED "modules/EmptyModule.cc")

add_executable(main "src/main.cc")
target_link_libraries(main momemta)
set_target_properties(main PROPERTIES OUTPUT_NAME "momemta.exe")
