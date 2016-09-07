# - Finds ROOT instalation
# This module sets up ROOT information
# It defines:
# ROOT_FOUND          If the ROOT is found
# ROOT_INCLUDE_DIR    PATH to the include directory
# ROOT_INCLUDE_DIRS   PATH to the include directories (not cached)
# ROOT_LIBRARIES      Most common libraries
# ROOT_<name>_LIBRARY Full path to the library <name>
# ROOT_LIBRARY_DIR    PATH to the library directory
# ROOT_DEFINITIONS    Compiler definitions and flags
#
# Updated by K. Smith (ksmith37@nd.edu) to properly handle
#  dependencies in ROOT_GENERATE_DICTIONARY

find_program(ROOT_CONFIG_EXECUTABLE root-config
  PATHS $ENV{ROOTSYS}/bin)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
    OUTPUT_VARIABLE ROOTSYS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --version
    OUTPUT_VARIABLE ROOT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIR})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libdir
    OUTPUT_VARIABLE ROOT_LIBRARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_LIBRARY_DIRS ${ROOT_LIBRARY_DIR})

set(rootlibs Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread)
set(ROOT_LIBRARIES)
foreach(_cpt ${rootlibs} ${ROOT_FIND_COMPONENTS})
  find_library(ROOT_${_cpt}_LIBRARY ${_cpt} HINTS ${ROOT_LIBRARY_DIR})
  if(ROOT_${_cpt}_LIBRARY)
    mark_as_advanced(ROOT_${_cpt}_LIBRARY)
    list(APPEND ROOT_LIBRARIES ${ROOT_${_cpt}_LIBRARY})
    list(REMOVE_ITEM ROOT_FIND_COMPONENTS ${_cpt})
  endif()
endforeach()
list(REMOVE_DUPLICATES ROOT_LIBRARIES)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --cflags
    OUTPUT_VARIABLE ROOT_DEFINITIONS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "(^|[ ]*)-I[^ ]*" "" ROOT_DEFINITIONS ${ROOT_DEFINITIONS})

execute_process(
  COMMAND ${ROOT_CONFIG_EXECUTABLE} --features
  OUTPUT_VARIABLE _root_options
  OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(_root_options)
foreach(_opt ${_root_options})
  set(ROOT_${_opt}_FOUND TRUE)
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ROOT DEFAULT_MSG ROOT_CONFIG_EXECUTABLE
    ROOTSYS ROOT_VERSION ROOT_INCLUDE_DIR ROOT_LIBRARIES ROOT_LIBRARY_DIR)

if (ROOT_FOUND AND NOT TARGET Root::Root)
    add_library(Root::Root INTERFACE IMPORTED)

    set_property(TARGET Root::Root PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${ROOT_INCLUDE_DIRS})
    set_property(TARGET Root::Root PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${ROOT_INCLUDE_DIRS})
    set_property(TARGET Root::Root PROPERTY INTERFACE_LINK_LIBRARIES ${ROOT_LIBRARIES})
endif()

mark_as_advanced(ROOT_CONFIG_EXECUTABLE)
