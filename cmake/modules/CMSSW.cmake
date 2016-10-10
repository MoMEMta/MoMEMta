# Find modules inside a CMSSW setup

set(IN_CMSSW OFF)

if(DEFINED ENV{CMSSW_VERSION})
    set(IN_CMSSW ON)
    message(STATUS "CMSSW environment found.")

    # Use scram to help cmake find dependencies

    # Find Boost
    if (NOT BOOST_ROOT AND "$ENV{BOOST_ROOT}" STREQUAL "")
        set(Boost_NO_SYSTEM_PATHS ON)
        execute_process(COMMAND scram tool tag boost BOOST_BASE OUTPUT_VARIABLE
            BOOST_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    # Find LHAPDF
    execute_process(COMMAND scram tool tag lhapdf LHAPDF_BASE OUTPUT_VARIABLE
        LHAPDF_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Find Python library
    execute_process(COMMAND scram tool tag python LIB OUTPUT_VARIABLE
        PYTHON_LIB OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND scram tool tag python LIBDIR OUTPUT_VARIABLE
        PYTHON_LIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(PYTHON_LIBRARY ${PYTHON_LIB_DIR}/lib${PYTHON_LIB}.so)

    execute_process(COMMAND scram tool tag python INCLUDE OUTPUT_VARIABLE
        PYTHON_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Find Python executable
    execute_process(COMMAND scram tool tag python PYTHON_BASE OUTPUT_VARIABLE
        PYTHON_BASE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(PYTHON_EXECUTABLE ${PYTHON_BASE_DIR}/bin/${PYTHON_LIB})
endif()
