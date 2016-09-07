add_subdirectory(dummy)

file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_library(matrix_elements INTERFACE)

foreach(child ${children})
    if(NOT IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
        continue()
    endif()

    # Ignore dummy
    if(${child} STREQUAL "dummy" OR ${child} STREQUAL "Cards")
        continue()
    endif()

    message(STATUS "Adding matrix element ${child}")
    add_subdirectory(${child})

    set(TARGET "me_${child}")

    # Set PIC
    set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)

    # Add MoMEMta include directories to the matrix element target
    target_include_directories(${TARGET} PRIVATE $<TARGET_PROPERTY:momemta,INTERFACE_INCLUDE_DIRECTORIES>)

    # And add sources of this matrix element to the MoMEMta target
    target_sources(matrix_elements INTERFACE $<TARGET_OBJECTS:${TARGET}>)
endforeach()
