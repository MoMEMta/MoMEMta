/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2016  Universite catholique de Louvain (UCL), Belgium
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <lua/Path.h>

#include <momemta/ILuaCallback.h>
#include <momemta/Path.h>

#include <lua/Types.h>

#include <lua.hpp>

void lua::path_register(lua_State* L, void* ptr) {

    // Register metatable for type Path
    push_type_metatable(L, LUA_PATH_TYPE_NAME);

    // Register destructor
    static const luaL_Reg functions[] = {
        {"__gc", path_free},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0); 

    lua_pop(L, 1);

    // Register global `Path` function, acting as a constructor for `Path` struct
    lua_pushlightuserdata(L, ptr);
    lua_pushcclosure(L, path_new, 1);
    lua_setglobal(L, LUA_PATH_TYPE_NAME);
}

int lua::path_new(lua_State* L) {

    int n = lua_gettop(L);
    if (n == 0) {
        luaL_error(L, "invalid number of arguments: at least 1 expected, got 0");
    }

    std::vector<std::string> module_names;
    for (size_t i = 1; i <= (size_t) n; i++) {
        std::string module_name = luaL_checkstring(L, i);
        module_names.push_back(module_name);
    }

    PathElementsPtr* pPath = static_cast<PathElementsPtr*>(lua_newuserdata(L, sizeof(PathElementsPtr)));
    *pPath = new PathElements();

    luaL_getmetatable(L, LUA_PATH_TYPE_NAME);
    lua_setmetatable(L, -2);

    (*pPath)->elements = module_names;

    
    void* cfg_ptr = lua_touserdata(L, lua_upvalueindex(1));
    ILuaCallback* callback = static_cast<ILuaCallback*>(cfg_ptr);
    callback->onNewPath(*pPath);

    return 1;
}

int lua::path_free(lua_State* L) {
    delete *static_cast<Path**>(luaL_checkudata(L, 1, LUA_PATH_TYPE_NAME));

    return 0;
}

PathElementsPtr lua::path_get(lua_State* L, int index) {
    luaL_checktype(L, index, LUA_TUSERDATA);
    PathElementsPtr* path = static_cast<PathElementsPtr*>(luaL_checkudata(L, index, LUA_PATH_TYPE_NAME));
    if (!path) {
        const char *msg = lua_pushfstring(L, "%s expected, got %s",
                LUA_PATH_TYPE_NAME, luaL_typename(L, index));
        luaL_argerror(L, index, msg);
    }

    return *path;
}
