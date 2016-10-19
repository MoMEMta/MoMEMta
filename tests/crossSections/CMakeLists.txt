### BlockA, p p > W+ W-, (W+ > e+ ve), (W- > mu- vm~)
### MadGraph: 0.867 +- 0.005 pb = 2.22(1)e-9 GeV^-2
add_executable(test_XS_blockA_WW_dilep "blockA_WW_dilep.cc")
target_link_libraries(test_XS_blockA_WW_dilep momemta)
set_target_properties(test_XS_blockA_WW_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockA_WW_dilep.exe")


### BlockB, p p > W+ W-, (W+ > e+ ve), (W- > mu- vm~)
### MadGraph: 0.867 +- 0.005 pb = 2.22(1)e-9 GeV^-2
add_executable(test_XS_blockB_WW_dilep "blockB_WW_dilep.cc")
target_link_libraries(test_XS_blockB_WW_dilep momemta)
set_target_properties(test_XS_blockB_WW_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockB_WW_dilep.exe")


### BlockD, pp > t t~, (t > w+ b, w+ > mu+ vm), (t~ > w- b~, w- > mu- vm~)
### MadGraph: 6.73 +- 0.02 pb = 1.73(5)e-8 GeV^-2
add_executable(test_XS_blockD_ttx_dilep "blockD_ttx_dilep.cc")
target_link_libraries(test_XS_blockD_ttx_dilep momemta)
set_target_properties(test_XS_blockD_ttx_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockD_ttx_dilep.exe")


### BlockF, p p > W+ W-, (W+ > e+ ve), (W- > mu- vm~)
### MadGraph: 0.867 +- 0.005 pb = 2.22(1)e-9 GeV^-2
add_executable(test_XS_blockF_WW_dilep "blockF_WW_dilep.cc")
target_link_libraries(test_XS_blockF_WW_dilep momemta)
set_target_properties(test_XS_blockF_WW_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockF_WW_dilep.exe")


### BlockG, p p > W+ W-, (W+ > e+ ve), (W- > mu- vm~)
### MadGraph: 0.867 +- 0.005 pb = 2.22(1)e-9 GeV^-2
add_executable(test_XS_blockG_WW_dilep "blockG_WW_dilep.cc")
target_link_libraries(test_XS_blockG_WW_dilep momemta)
set_target_properties(test_XS_blockG_WW_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockG_WW_dilep.exe")


### BlockB + SecondaryBlockCD, p p > W+ W-, (W+ > e+ ve), (W- > mu- vm~)
### MadGraph: 0.867 +- 0.005 pb = 2.22(1)e-9 GeV^-2
add_executable(test_XS_blockB_secondaryBlockCD_WW_dilep "blockB_secondaryBlockCD_WW_dilep.cc")
target_link_libraries(test_XS_blockB_secondaryBlockCD_WW_dilep momemta)
set_target_properties(test_XS_blockB_secondaryBlockCD_WW_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockB_secondaryBlockCD_WW_dilep.exe")


