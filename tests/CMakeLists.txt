# Test executables

add_executable(test_psVol_blockD "phaseSpaceVolume/blockD.cc")
target_link_libraries(test_psVol_blockD momemta)
set_target_properties(test_psVol_blockD PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume_blockD.exe")
