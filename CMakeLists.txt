cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)
project(MoMEMta)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)
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
set(PYTHON_MIN_VERSION "2.6" CACHE STRING "Minimum Python version to look for")
set(Boost_PYTHON_VERSION_TAG "" CACHE STRING "Suffix for boost::python shared library (X in libboost_pythonX.py)")

option(DEBUG_TIMING "Debug modules runtime. After each weight computation, a summry of each module runtime is printed" OFF)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Require c++11 *at least*, use default compiler standard if possible
if (CMAKE_CXX_STANDARD_COMPUTED_DEFAULT STRLESS "11" OR
        CMAKE_CXX_STANDARD_COMPUTED_DEFAULT STREQUAL "98")
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

# Stick to the standard
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS OFF)

# Default compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcx-fortran-rules -fcx-limited-range")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -ffast-math -Wall")

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
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g")

# Find dependencies
include(CMSSW)
find_package(ROOT 6.20.00 REQUIRED)
find_package(LHAPDF 6.0 REQUIRED)

if (NOT USE_BUILTIN_LUA)
    find_package(Lua 5.3 QUIET)
endif()

set(Boost_NO_BOOST_CMAKE ON)

# Find boost headers
find_package(Boost 1.54 REQUIRED)
if (NOT TARGET Boost)
    add_library(Boost INTERFACE IMPORTED)
    set_property(TARGET Boost PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
    set_property(TARGET Boost PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
endif()

if (PYTHON_BINDINGS)
    find_package(PythonInterp ${PYTHON_MIN_VERSION})
    if (PYTHONINTERP_FOUND)

        find_package(PythonLibs ${PYTHON_MIN_VERSION})

        if (NOT TARGET Python::libs)
            add_library(Python::libs INTERFACE IMPORTED)
            set_property(TARGET Python::libs PROPERTY INTERFACE_LINK_LIBRARIES ${PYTHON_LIBRARIES})
            set_property(TARGET Python::libs PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_PATH})
            set_property(TARGET Python::libs PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_PATH})
        endif()

        string(CONCAT BP_NAME_LC "python" "${Boost_PYTHON_VERSION_TAG}")
        string(TOUPPER ${BP_NAME_LC} BP_NAME_UC)

        find_package(Boost 1.54 COMPONENTS ${BP_NAME_LC})
        if (NOT Boost_${BP_NAME_UC}_FOUND)
            message(WARNING "Boost ${BP_NAME_LC} not found. Not building python bindings")
            set(PYTHON_BINDINGS OFF)
        elseif(NOT TARGET Boost::${BP_NAME_LC})
            add_library(Boost::${BP_NAME_LC} INTERFACE IMPORTED)
            set_property(TARGET Boost::${BP_NAME_LC} PROPERTY
                INTERFACE_LINK_LIBRARIES ${Boost_${BP_NAME_UC}_LIBRARY})
            set_property(TARGET Boost::${BP_NAME_LC} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
            set_property(TARGET Boost::${BP_NAME_LC} PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
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

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/momemta/config.h
    )

set(MOMEMTA_SOURCES
    "modules/BinnedTransferFunctionOnEnergy.cc"
    "modules/BinnedTransferFunctionOnPt.cc"
    "modules/BlockA.cc"
    "modules/BlockB.cc"
    "modules/BlockC.cc"
    "modules/BlockD.cc"
    "modules/BlockE.cc"
    "modules/BlockF.cc"
    "modules/BlockG.cc"
    "modules/InternalModules.cc"
    "modules/SecondaryBlockA.cc"
    "modules/SecondaryBlockB.cc"
    "modules/SecondaryBlockCD.cc"
    "modules/SecondaryBlockE.cc"
    "modules/BuildInitialState.cc"
    "modules/BreitWignerGenerator.cc"
    "modules/Constant.cc"
    "modules/Counter.cc"
    "modules/DMEM.cc"
    "modules/FlatTransferFunctionOnP.cc"
    "modules/FlatTransferFunctionOnPhi.cc"
    "modules/FlatTransferFunctionOnTheta.cc"
    "modules/GaussianTransferFunctionOnEnergy.cc"
    "modules/GaussianTransferFunctionOnPt.cc"
    "modules/Looper.cc"
    "modules/LooperSummer.cc"
    "modules/MatrixElement.cc"
    "modules/NarrowWidthApproximation.cc"
    "modules/Permutator.cc"
    "modules/Printer.cc"
    "modules/StandardPhaseSpace.cc"
    "modules/UniformGenerator.cc"
    "modules/LinearCombinator.cc"
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
    "core/src/ModuleUtils.cc"
    "core/src/MEParameters.cc"
    "core/src/ParameterSet.cc"
    "core/src/Particle.cc"
    "core/src/Path.cc"
    "core/src/Pool.cc"
    "core/src/SharedLibrary.cc"
    "core/src/SLHAReader.cc"
    "core/src/Solution.cc"
    "core/src/Utils.cc"
    "core/src/lib/optional.cc"
    "core/src/logger/formatter.cc"
    "core/src/logger/logger.cc"
    "core/src/logger/sink.cc"
    "core/src/lua/LazyTable.cc"
    "core/src/lua/ParameterSetParser.cc"
    "core/src/lua/utils.cc"
    "core/src/lua/bindings/Path.cc"
    "core/src/lua/bindings/Types.cc"
    "core/src/registry/ModuleDefBuilder.cc"
    "core/src/registry/ModuleDefUtils.cc"
    "core/src/registry/ModuleRegistry.cc"
    "core/src/strings/Scanner.cc"
    "core/src/strings/StringPiece.cc"
    )

# Embed lua scripts into the C++ code
file(GLOB LUA_FILES
        "${CMAKE_CURRENT_LIST_DIR}/lua/*.lua"
        )
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/lua_code.cc
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/scripts/embedLua.py
            ARGS -i ${CMAKE_CURRENT_LIST_DIR}/lua -o ${CMAKE_CURRENT_BINARY_DIR}/generated/lua_code.cc
        DEPENDS
            ${CMAKE_CURRENT_LIST_DIR}/scripts/embedLua.py
            ${LUA_FILES}
        COMMENT "Embedding lua code..."
)
list(APPEND MOMEMTA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/generated/lua_code.cc)

add_library(momemta SHARED ${MOMEMTA_SOURCES})
set_target_properties(momemta PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Public interface
target_include_directories(momemta PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")

# Private interface
target_include_directories(momemta PRIVATE "${CMAKE_CURRENT_LIST_DIR}/core/include")

# Libraries
target_link_libraries(momemta PRIVATE cuba)
target_link_libraries(momemta PRIVATE lua)
target_link_libraries(momemta PRIVATE LHAPDF::LHAPDF)
target_link_libraries(momemta PRIVATE Boost)

target_link_libraries(momemta PUBLIC dl)
target_link_libraries(momemta PUBLIC ROOT::Core ROOT::Tree ROOT::Hist ROOT::MathCore)

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

    target_link_libraries(momemta_python PRIVATE Python::libs
        Boost::${BP_NAME_LC} Root::python momemta)
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
install(TARGETS momemta EXPORT momemta_targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include)

if(PYTHON_BINDINGS)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig; print(distutils.sysconfig.get_python_lib(prefix='', plat_specific=True))"
            OUTPUT_VARIABLE PYTHON_INSTALL_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

    install(TARGETS momemta_python
            LIBRARY DESTINATION ${PYTHON_INSTALL_PREFIX})
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/MoMEMtaConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

set(ConfigPackageLocation lib/cmake/MoMEMta)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/MoMEMtaConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/MoMEMtaConfig.cmake"
        INSTALL_DESTINATION "${ConfigPackageLocation}"
        )

install(EXPORT momemta_targets
        FILE
            MoMEMtaTargets.cmake
        NAMESPACE
            momemta::
        DESTINATION
            ${ConfigPackageLocation}
        )

export(TARGETS momemta
       FILE
           MoMEMtaTargets.cmake
       NAMESPACE
           momemta::
       )

install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/MoMEMtaConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/MoMEMtaConfigVersion.cmake"
        DESTINATION
            ${ConfigPackageLocation}
)

if (IN_CMSSW)
    # Tool XML description
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/cmssw_tool.xml.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmssw_tool.xml"
        IMMEDIATE @ONLY)

    install(CODE "execute_process(COMMAND scram setup \"${CMAKE_CURRENT_BINARY_DIR}/cmssw_tool.xml\")")
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
find_package(Doxygen 1.8.12)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/theme/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/tools/make_docs.sh.in ${CMAKE_CURRENT_BINARY_DIR}/make_docs.sh @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
endif(DOXYGEN_FOUND)
