SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_subdirectory(pp_ttx_fully_leptonic)
add_subdirectory(example_WW_llvv)
add_subdirectory(dummy)
