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

#include <lua/Types.h>

#include <momemta/Path.h>

#include <lua/Path.h>
#include <lua.hpp>

void push_type_metatable(lua_State* L, const char* name) {
    // Register metatable
    luaL_newmetatable(L, name);

    // Add '__type_' field
    lua_pushstring(L, name);
    lua_setfield(L, -2, "__type");
}

std::string get_custom_type_name(lua_State* L, int index) {
    if (lua_getmetatable(L, index)) {
        lua_getfield(L, -1, "__type");
        const char* type = lua_tostring(L, -1);
        lua_pop(L, 2);

        return type;
    }

    luaL_error(L, "No metatable associated with index %d", index);
    return "";
}

boost::any get_custom_type_ptr(lua_State* L, int index) {
    std::string type = get_custom_type_name(L, index);

    boost::any result;
    if (type == LUA_PATH_TYPE_NAME) {
        Path path = Path(lua::path_get(L, index));
        result = path;
    }

    if (result.empty()) {
        luaL_error(L, "Invalid userdata type: %s", type.c_str());
    }

    return result;
}
