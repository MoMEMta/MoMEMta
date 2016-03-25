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

# Find dependencies
find_package(ROOT REQUIRED)
find_package(LHAPDF 6.0 REQUIRED)

if (NOT USE_BUILTIN_LUA)
    find_package(Lua QUIET)
endif()

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
    "modules/BlockD.cc"
    "modules/Boost.cc"
    "modules/Flatter.cc"
    "modules/GaussianTransferFunction.cc"
    "modules/MatrixElement.cc"
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O2 -fcx-fortran-rules -fcx-limited-range")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${EXTERNAL_DIR})
include_directories(${EXTERNAL_DIR}/cuba)
include_directories(${EXTERNAL_DIR}/spdlog)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${LHAPDF_INCLUDE_DIRS})
include_directories(${ROOT_INCLUDE_DIR})

target_link_libraries(momemta LINK_PRIVATE dl)
target_link_libraries(momemta LINK_PRIVATE cuba)
target_link_libraries(momemta LINK_PRIVATE ${LUA_LIBRARIES})
target_link_libraries(momemta LINK_PRIVATE pthread)
target_link_libraries(momemta LINK_PRIVATE ${LHAPDF_LIBRARIES})
target_link_libraries(momemta LINK_PRIVATE ${ROOT_LIBRARIES})

find_library(ROOT_GENVECTOR_LIBRARY GenVector HINTS ${ROOT_LIBRARY_DIR})
target_link_libraries(momemta LINK_PRIVATE ${ROOT_GENVECTOR_LIBRARY})

# Matrix elements
add_subdirectory(MatrixElements)
target_link_libraries(momemta LINK_PRIVATE me_pp_ttx_fully_leptonic)
target_link_libraries(momemta LINK_PRIVATE "-Wl,--whole-archive $<TARGET_FILE:me_pp_ttx_fully_leptonic> -Wl,--no-whole-archive")

add_library(empty_module SHARED "modules/EmptyModule.cc")

add_executable(main "core/src/main.cc")
target_link_libraries(main momemta)
set_target_properties(main PROPERTIES OUTPUT_NAME "momemta.exe")

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
