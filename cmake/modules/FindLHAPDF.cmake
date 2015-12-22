# Try to find LHAPDF using `lhapdf-config`
# This will define
#     LHAPDF_FOUND           - System has LHAPDF installed
#     LHAPDF_INCLUDE_DIRS    - LHAPDF include directories
#     LHAPDF_LIBRARIES       - LHAPDF library

find_program(LHAPDF_CONFIG_PROGRAM lhapdf-config)

if (LHAPDF_CONFIG_PROGRAM)
    exec_program( ${LHAPDF_CONFIG_PROGRAM} ARGS "--incdir" OUTPUT_VARIABLE LHAPDF_INCLUDE_DIRS )
    exec_program( ${LHAPDF_CONFIG_PROGRAM} ARGS "--libdir" OUTPUT_VARIABLE LHAPDF_LIBRARY_DIR )

    find_library( LHAPDF_LIBRARY LHAPDF HINTS ${LHAPDF_LIBRARY_DIR} )
    
    set( LHAPDF_LIBRARIES ${LHAPDF_LIBRARY} )

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LHAPDF DEFAULT_MSG LHAPDF_LIBRARIES LHAPDF_INCLUDE_DIRS)    

mark_as_advanced(LHAPDF_INCLUDE_DIRS LHAPDF_LIBRARY_DIR LHAPDF_LIBRARY )
