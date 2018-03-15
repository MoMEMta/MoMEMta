set(SOURCES
    "integrand.cc"
    "no_integration.cc"
    "integration_tests.cc"
    )

add_executable(integration_tests ${SOURCES})

target_link_libraries(integration_tests momemta)

set_target_properties(integration_tests PROPERTIES OUTPUT_NAME
      "integration_tests.exe")
