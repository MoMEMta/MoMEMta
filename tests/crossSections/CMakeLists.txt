### BlockD, pp > t t~, (t > w+ b, w+ > mu+ vm), (t~ > w- b~, w- > mu- vm~) == 6.73 +- 0.02 pb (MadGraph)

add_executable(test_XS_blockD_ttx_dilep "blockD_ttx_dilep.cc")
target_link_libraries(test_XS_blockD_ttx_dilep momemta)
set_target_properties(test_XS_blockD_ttx_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockD_ttx_dilep.exe")


### BlockF
add_executable(test_XS_blockF_WW_dilep "blockF_WW_dilep.cc")
target_link_libraries(test_XS_blockF_WW_dilep momemta)
set_target_properties(test_XS_blockF_WW_dilep PROPERTIES OUTPUT_NAME
  "crossSection_blockF_WW_dilep.exe")
