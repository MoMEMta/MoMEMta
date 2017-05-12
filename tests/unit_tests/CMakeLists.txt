set(SOURCES
    "graph.cc"
    "lua.cc"
    "modules.cc"
    "ParameterSet.cc"
    "pool.cc"
    "unit_tests.cc"
    "lib/optional.cc"
    "strings/Scanner.cc"
    )

add_executable(unit_tests ${SOURCES})

target_link_libraries(unit_tests momemta)
target_link_libraries(unit_tests lua)

# Add dependency to the empty module target, used by unit test to ensure dynamic loading
# of modules work
add_dependencies(unit_tests empty_module)

# Add private include directories from MoMEMta
target_include_directories(unit_tests PRIVATE $<TARGET_PROPERTY:momemta,INCLUDE_DIRECTORIES>)

set_target_properties(unit_tests PROPERTIES OUTPUT_NAME
      "unit_tests.exe")
