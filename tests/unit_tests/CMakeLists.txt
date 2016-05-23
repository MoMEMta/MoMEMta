set(SOURCES
    "lua.cc"
    "ParameterSet.cc"
    "unit_tests.cc"
    )

add_executable(unit_tests ${SOURCES})
target_link_libraries(unit_tests ${LUA_LIBRARIES})
target_link_libraries(unit_tests momemta)
set_target_properties(unit_tests PROPERTIES OUTPUT_NAME
      "unit_tests.exe")
