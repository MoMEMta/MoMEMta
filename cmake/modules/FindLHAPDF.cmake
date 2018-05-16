# Try to find LHAPDF using `lhapdf-config`
# This will define
#     LHAPDF_FOUND           - System has LHAPDF installed
#     LHAPDF_INCLUDE_DIRS    - LHAPDF include directories
#     LHAPDF_LIBRARIES       - LHAPDF library
#     LHAPDF_VERSION_STRING  - LHAPDF version

# First try to look for LHAPDF.h in standard path
# lhapdf-config is not a reliable way to find lhapdf

find_path(LHAPDF_INCLUDE_DIR LHAPDF/LHAPDF.h HINTS ${LHAPDF_ROOT}
    PATH_SUFFIXES "include")
find_library(LHAPDF_LIBRARY LHAPDF HINTS ${LHAPDF_ROOT} PATH_SUFFIXES lib)

if (NOT LHAPDF_INCLUDE_DIR OR NOT LHAPDF_LIBRARY)
    # Not found, try to look for lhapdf-config
    find_program(LHAPDF_CONFIG_PROGRAM lhapdf-config)

    if (LHAPDF_CONFIG_PROGRAM)
        exec_program(${LHAPDF_CONFIG_PROGRAM} ARGS "--incdir" OUTPUT_VARIABLE LHAPDF_INCLUDE_DIR)
        exec_program(${LHAPDF_CONFIG_PROGRAM} ARGS "--libdir" OUTPUT_VARIABLE LHAPDF_LIBRARY_DIR)
    endif()

    find_library(LHAPDF_LIBRARY LHAPDF HINTS ${LHAPDF_LIBRARY_DIR})
endif()

if (LHAPDF_INCLUDE_DIR)
    include(CheckIncludeFileCXX)
    set(CMAKE_REQUIRED_INCLUDES ${LHAPDF_INCLUDE_DIR})
    check_include_file_cxx(LHAPDF/Version.h LHAPDF_HAVE_VERSION) 
endif()

if (LHAPDF_HAVE_VERSION)
    # Read LHAPDF version

    #define LHAPDF_VERSION "6.1.6"
    file(STRINGS "${LHAPDF_INCLUDE_DIR}/LHAPDF/Version.h"
        LHAPDF_VERSION_STRING
        REGEX "^#define[ \t]+LHAPDF_VERSION[ \t]+\"([0-9]+\\.[0-9]+\\.[0-9]+).*"
        )

    string(REGEX REPLACE
        "^#define[ \t]+LHAPDF_VERSION[ \t]+\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*"
        "\\1"
        LHAPDF_VERSION_STRING
        ${LHAPDF_VERSION_STRING}
        )
endif()

set(LHAPDF_INCLUDE_DIRS ${LHAPDF_INCLUDE_DIR})
set(LHAPDF_LIBRARIES ${LHAPDF_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LHAPDF
    FOUND_VAR LHAPDF_FOUND
    REQUIRED_VARS LHAPDF_LIBRARIES LHAPDF_INCLUDE_DIRS LHAPDF_HAVE_VERSION
    VERSION_VAR LHAPDF_VERSION_STRING
    )

if (LHAPDF_FOUND AND NOT TARGET LHAPDF::LHAPDF)
    add_library(LHAPDF::LHAPDF INTERFACE IMPORTED)

    set_property(TARGET LHAPDF::LHAPDF PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LHAPDF_INCLUDE_DIRS})
    set_property(TARGET LHAPDF::LHAPDF PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${LHAPDF_INCLUDE_DIRS})
    set_property(TARGET LHAPDF::LHAPDF PROPERTY INTERFACE_LINK_LIBRARIES ${LHAPDF_LIBRARIES})
endif()

mark_as_advanced(ROOT_CONFIG_EXECUTABLE)

mark_as_advanced(LHAPDF_INCLUDE_DIRS LHAPDF_LIBRARIES LHAPDF_VERSION_STRING)
