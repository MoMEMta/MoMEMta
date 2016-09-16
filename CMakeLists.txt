cmake_minimum_required(VERSION 3.2.1 FATAL_ERROR)
project(MoMEMta)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Options
option(USE_BUILTIN_LUA "Force usage of built-in lua interpreter, even if a system-wide version if available" OFF)

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

option(COVERAGE "Enable code coverage" OFF)

option(PYTHON_BINDINGS "Build python bindings. Python needs to be available" OFF)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Default compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -fcx-fortran-rules -fcx-limited-range -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -ffast-math -Wall --std=c11")

if (COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

# If profiling, always include debug information
if (PROFILING)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
endif()

# Compiler flags for each build type
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g")

# Find dependencies
find_package(ROOT 5.34.09 REQUIRED)
find_package(LHAPDF 6.0 REQUIRED)
find_package(Threads REQUIRED)

if (NOT USE_BUILTIN_LUA)
    find_package(Lua 5.3 QUIET)
endif()

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.54 REQUIRED log)

if (NOT TARGET Boost::log)
    add_library(Boost::log INTERFACE IMPORTED)
    set_property(TARGET Boost::log PROPERTY INTERFACE_LINK_LIBRARIES ${Boost_LOG_LIBRARY})
    set_property(TARGET Boost::log PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
    set_property(TARGET Boost::log PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
endif()

if (PYTHON_BINDINGS)
    find_package(PythonLibs 2.6)
    if (PYTHONLIBS_FOUND)

        if (NOT TARGET Python::libs)
            add_library(Python::libs INTERFACE IMPORTED)
            set_property(TARGET Python::libs PROPERTY INTERFACE_LINK_LIBRARIES ${PYTHON_LIBRARIES})
            set_property(TARGET Python::libs PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_PATH})
            set_property(TARGET Python::libs PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_PATH})
        endif()

        find_package(PythonInterp 2.6)

        find_package(Boost 1.54 COMPONENTS python)
        if (NOT Boost_PYTHON_FOUND)
            message(WARNING "Boost python not found. Not building python bindings")
            set(PYTHON_BINDINGS OFF)
        elseif(NOT TARGET Boost::python)
            add_library(Boost::python INTERFACE IMPORTED)
            set_property(TARGET Boost::python PROPERTY INTERFACE_LINK_LIBRARIES ${Boost_PYTHON_LIBRARY})
            set_property(TARGET Boost::python PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
            set_property(TARGET Boost::python PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
        endif()

    else()
        message(WARNING "Python not found. Not building python bindings")
        set(PYTHON_BINDINGS OFF)
    endif()

endif()

if (PYTHON_BINDINGS)
    message(STATUS "Building python bindings")

    # Look for pyroot, optional
    find_library(ROOT_PYROOT_LIBRARY PyROOT HINTS ${ROOT_LIBRARY_DIR})
    add_library(Root::python INTERFACE IMPORTED)
    if (ROOT_PYROOT_LIBRARY)
        set_property(TARGET Root::python APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "ROOT_HAS_PYROOT")
        set_property(TARGET Root::python APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${ROOT_PYROOT_LIBRARY})
    endif()
endif()

# Include external
add_subdirectory(external)

if (NOT LUA_FOUND)
    if (USE_BUILTIN_LUA)
        message(STATUS "Using built-in lua interpreter")
    else()
        message(STATUS "No system-wide Lua found. Using built-in distribution")
    endif()
else()
    message(STATUS "Found Lua: ${LUA_LIBRARIES} (found suitable version \"${LUA_VERSION_STRING}\", minimum required is \"5.3\")")
    add_library(lua INTERFACE IMPORTED)
    set_property(TARGET lua PROPERTY INTERFACE_LINK_LIBRARIES ${LUA_LIBRARIES})
    set_property(TARGET lua PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR})
    set_property(TARGET lua PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR})
endif()

set(MOMEMTA_SOURCES
    "modules/BinnedTransferFunctionOnEnergy.cc"
    "modules/BlockA.cc"
    "modules/BlockB.cc"
    "modules/BlockD.cc"
    "modules/BlockF.cc"
    "modules/SecondaryBlockCD.cc"
    "modules/BuildInitialState.cc"
    "modules/BreitWignerGenerator.cc"
    "modules/Constant.cc"
    "modules/Counter.cc"
    "modules/DMEM.cc"
    "modules/FlatTransferFunctionOnP.cc"
    "modules/FlatTransferFunctionOnPhi.cc"
    "modules/FlatTransferFunctionOnTheta.cc"
    "modules/GaussianTransferFunctionOnEnergy.cc"
    "modules/Looper.cc"
    "modules/MatrixElement.cc"
    "modules/NarrowWidthApproximation.cc"
    "modules/Permutator.cc"
    "modules/Printer.cc"
    "modules/StandardPhaseSpace.cc"
    "modules/Summer.cc"
    "modules/UniformGenerator.cc"
    "core/src/Configuration.cc"
    "core/src/ConfigurationReader.cc"
    "core/src/Graph.cc"
    "core/src/InputTag.cc"
    "core/src/LibraryManager.cc"
    "core/src/logging.cc"
    "core/src/Math.cc"
    "core/src/MatrixElementFactory.cc"
    "core/src/MoMEMta.cc"
    "core/src/Module.cc"
    "core/src/ModuleFactory.cc"
    "core/src/MEParameters.cc"
    "core/src/ParameterSet.cc"
    "core/src/Path.cc"
    "core/src/Pool.cc"
    "core/src/SharedLibrary.cc"
    "core/src/SLHAReader.cc"
    "core/src/Solution.cc"
    "core/src/Utils.cc"
    "core/src/lua/Path.cc"
    "core/src/lua/Types.cc"
    "core/src/lua/utils.cc"
    )

add_library(momemta SHARED ${MOMEMTA_SOURCES})
set_target_properties(momemta PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Public interface
target_include_directories(momemta PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

# Private interface
target_include_directories(momemta PRIVATE "${CMAKE_CURRENT_LIST_DIR}/core/include")

# Libraries
target_link_libraries(momemta PRIVATE cuba)
target_link_libraries(momemta PRIVATE lua)
target_link_libraries(momemta PRIVATE LHAPDF::LHAPDF)

target_link_libraries(momemta PUBLIC dl)
target_link_libraries(momemta PUBLIC Root::Root)
target_link_libraries(momemta PUBLIC Boost::log)
target_link_libraries(momemta PUBLIC Threads::Threads)

find_library(ROOT_GENVECTOR_LIBRARY GenVector HINTS ${ROOT_LIBRARY_DIR})
target_link_libraries(momemta PUBLIC ${ROOT_GENVECTOR_LIBRARY})

if (PROFILING)
    target_link_libraries(momemta PUBLIC "-Wl,--no-as-needed ${GPERF_PROFILER_LIBRARY} -Wl,--as-needed")
    target_link_libraries(momemta PUBLIC "-Wl,--no-as-needed ${GPERF_TCMALLOC_LIBRARY} -Wl,--as-needed")
endif()

# Matrix elements
add_subdirectory(MatrixElements)
target_link_libraries(momemta PRIVATE matrix_elements)

add_library(empty_module SHARED "modules/EmptyModule.cc")
target_link_libraries(empty_module momemta)

# Bindings

if (PYTHON_BINDINGS)
    set(BINDING_SOURCES
            core/src/python/bindings.cc
    )

    add_library(momemta_python SHARED ${BINDING_SOURCES})

    target_link_libraries(momemta_python PRIVATE Python::libs Boost::python Root::python momemta)
    set_target_properties(momemta_python PROPERTIES PREFIX "")
    set_target_properties(momemta_python PROPERTIES OUTPUT_NAME "momemta")
endif()

# Example executables

option(EXAMPLES "Compile examples" ON)

if(EXAMPLES)
    add_executable(example_tt_fullyleptonic "examples/tt_fullyleptonic.cc")
    target_link_libraries(example_tt_fullyleptonic momemta)
    set_target_properties(example_tt_fullyleptonic PROPERTIES OUTPUT_NAME
      "tt_fullyleptonic.exe")

    add_executable(example_tt_fullyleptonic_NWA "examples/tt_fullyleptonic_NWA.cc")
    target_link_libraries(example_tt_fullyleptonic_NWA momemta)
    set_target_properties(example_tt_fullyleptonic_NWA PROPERTIES OUTPUT_NAME
      "tt_fullyleptonic_NWA.exe")

    add_executable(example_WW_fullyleptonic "examples/WW_fullyleptonic.cc")
    target_link_libraries(example_WW_fullyleptonic momemta)
    set_target_properties(example_WW_fullyleptonic PROPERTIES OUTPUT_NAME
      "WW_fullyleptonic.exe")

endif()

# Test executables
option(TESTS "Compile tests" OFF)

if(TESTS)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/run_tests.sh.in"
            "${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh"
            IMMEDIATE @ONLY)

    add_subdirectory(tests)
endif()

# Install targets

# First, headers
install(DIRECTORY include/ DESTINATION include)
# And MoMEMta library
install(TARGETS momemta
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

if(PYTHON_BINDINGS)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig; print(distutils.sysconfig.get_python_lib(prefix='', plat_specific=True))"
            OUTPUT_VARIABLE PYTHON_INSTALL_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

    install(TARGETS momemta_python
            LIBRARY DESTINATION ${PYTHON_INSTALL_PREFIX})
endif()

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
