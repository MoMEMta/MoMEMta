# Phase space volume integration
add_executable(test_psVol "phaseSpaceVolume.cc" "phaseSpaceGenerationTests.cc")
target_compile_definitions(test_psVol PRIVATE
    SOURCE_PATH="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_psVol momemta)
set_target_properties(test_psVol PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume.exe")

# Cross section integration integration
add_executable(test_XS "crossSections.cc" "phaseSpaceGenerationTests.cc")
target_compile_definitions(test_XS PRIVATE
    SOURCE_PATH="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_XS momemta)
set_target_properties(test_XS PROPERTIES OUTPUT_NAME
  "crossSections.exe")
