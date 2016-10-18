### BlockA

add_executable(test_psVol_blockA "blockA.cc")
target_link_libraries(test_psVol_blockA momemta)
set_target_properties(test_psVol_blockA PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume_blockA.exe")

### BlockB

add_executable(test_psVol_blockB "blockB.cc")
target_link_libraries(test_psVol_blockB momemta)
set_target_properties(test_psVol_blockB PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume_blockB.exe")

### BlockD

add_executable(test_psVol_blockD "blockD.cc")
target_link_libraries(test_psVol_blockD momemta)
set_target_properties(test_psVol_blockD PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume_blockD.exe")

### BlockF

add_executable(test_psVol_blockF "blockF.cc")
target_link_libraries(test_psVol_blockF momemta)
set_target_properties(test_psVol_blockF PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume_blockF.exe")

### BlockG

add_executable(test_psVol_blockG "blockG.cc")
target_link_libraries(test_psVol_blockG momemta)
set_target_properties(test_psVol_blockG PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume_blockG.exe")

### BlockA + SecondaryBlockCD

add_executable(test_psVol_blockA_secondaryBlockCD "blockA_secondaryBlockCD.cc")
target_link_libraries(test_psVol_blockA_secondaryBlockCD momemta)
set_target_properties(test_psVol_blockA_secondaryBlockCD PROPERTIES OUTPUT_NAME
  "phaseSpaceVolume_blockA_secondaryBlockCD.exe")

