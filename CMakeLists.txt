cmake_minimum_required(VERSION 2.8.8)
project(MoMEMta)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Public interface
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

# Private interface
include_directories("${CMAKE_CURRENT_LIST_DIR}/core/include")

# Options
option(USE_BUILTIN_LUA "Force usage of built-in lua interpreter, even if a system-wide version if available" OFF)
option(TRACE_LOGGING "Set logging threshold to TRACE instead of DEBUG" OFF)
if (TRACE_LOGGING)
    add_definitions(-DSPDLOG_TRACE_ON)
endif()

option(PROFILING "Enable profiling" OFF)
if (PROFILING)
    add_definitions(-DPROFILING)

    # Look for libprofiler
    find_library(GPERF_PROFILER_LIBRARY profiler)
    find_library(GPERF_TCMALLOC_LIBRARY tcmalloc_minimal)
    if (GPERF_PROFILER_LIBRARY AND GPERF_TCMALLOC_LIBRARY)
        message(STATUS "Found gperftools: ${GPERF_PROFILER_LIBRARY} ${GPERF_TCMALLOC_LIBRARY}")
    else()
        message(STATUS "gperftools NOT found: profiling disabled")
        set(PROFILING OFF)
    endif()
endif()

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Default compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra --std=c++11 -fcx-fortran-rules -fcx-limited-range")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -ffast-math -Wall --std=c11")

# If profiling, always include debug information
if (PROFILING)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
endif()

# Compiler flags for each build type
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_C_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g")

# Find dependencies
find_package(ROOT REQUIRED)
find_package(LHAPDF 6.0 REQUIRED)

if (NOT USE_BUILTIN_LUA)
    find_package(Lua QUIET)
endif()

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED)

# Include external
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/external")
add_subdirectory(external)

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
    "modules/BlockB.cc"
    "modules/BlockD.cc"
    "modules/Boost.cc"
    "modules/Flatter.cc"
    "modules/GaussianTransferFunction.cc"
    "modules/BinnedTransferFunctionOnEnergy.cc"
    "modules/MatrixElement.cc"
    "modules/Permutator.cc"
    "core/src/ConfigurationReader.cc"
    "core/src/ConfigurationSet.cc"
    "core/src/InputTag.cc"
    "core/src/LibraryManager.cc"
    "core/src/logging.cc"
    "core/src/MatrixElementFactory.cc"
    "core/src/MoMEMta.cc"
    "core/src/ModuleFactory.cc"
    "core/src/Pool.cc"
    "core/src/SharedLibrary.cc"
    "core/src/SLHAReader.cc"
    "core/src/Utils.cc"
    "core/src/lua/utils.cc"
    )

add_library(momemta SHARED ${MOMEMTA_SOURCES})
set_target_properties(momemta PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(SYSTEM ${EXTERNAL_DIR})
include_directories(SYSTEM ${EXTERNAL_DIR}/cuba)
include_directories(SYSTEM ${EXTERNAL_DIR}/spdlog)
include_directories(SYSTEM ${LUA_INCLUDE_DIR})
include_directories(SYSTEM ${LHAPDF_INCLUDE_DIRS})
include_directories(SYSTEM ${ROOT_INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

target_link_libraries(momemta LINK_PUBLIC dl)
target_link_libraries(momemta LINK_PRIVATE cuba)
target_link_libraries(momemta LINK_PRIVATE ${LUA_LIBRARIES})
target_link_libraries(momemta LINK_PUBLIC pthread)
target_link_libraries(momemta LINK_PUBLIC ${LHAPDF_LIBRARIES})
target_link_libraries(momemta LINK_PUBLIC ${ROOT_LIBRARIES})

find_library(ROOT_GENVECTOR_LIBRARY GenVector HINTS ${ROOT_LIBRARY_DIR})
target_link_libraries(momemta LINK_PUBLIC ${ROOT_GENVECTOR_LIBRARY})

if (PROFILING)
    target_link_libraries(momemta LINK_PUBLIC "-Wl,--no-as-needed ${GPERF_PROFILER_LIBRARY} -Wl,--as-needed")
    target_link_libraries(momemta LINK_PUBLIC "-Wl,--no-as-needed ${GPERF_TCMALLOC_LIBRARY} -Wl,--as-needed")
endif()

# Matrix elements
add_subdirectory(MatrixElements)
target_link_libraries(momemta LINK_PRIVATE me_pp_ttx_fully_leptonic)
target_link_libraries(momemta LINK_PRIVATE "-Wl,--whole-archive $<TARGET_FILE:me_pp_ttx_fully_leptonic> -Wl,--no-whole-archive")

add_library(empty_module SHARED "modules/EmptyModule.cc")

add_executable(example_tt_fullyleptonic "examples/tt_fullyleptonic.cc")
target_link_libraries(example_tt_fullyleptonic momemta)
set_target_properties(example_tt_fullyleptonic PROPERTIES OUTPUT_NAME
  "tt_fullyleptonic.exe")

# Install targets

# First, headers
install(DIRECTORY include/ DESTINATION include)
# And MoMEMta library
install(TARGETS momemta
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

# Uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Documentation

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/theme/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/tools/make_docs.sh.in ${CMAKE_CURRENT_BINARY_DIR}/make_docs.sh @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
endif(DOXYGEN_FOUND)
