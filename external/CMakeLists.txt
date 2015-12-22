add_subdirectory(cuba)

if (NOT LUA_FOUND)
    add_subdirectory(lua)
endif()
