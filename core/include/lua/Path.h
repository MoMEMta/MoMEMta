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

#pragma once

#include <string>
#include <vector>

struct lua_State;
struct PathElements;

/**
 * \file
 * \brief Lua binding of C++ Path class
 */

#define LUA_PATH_TYPE_NAME "Path"

namespace lua {

/**
 * \brief Register Path into lua runtime
 */
void path_register(lua_State* L, void* ptr);

/**
 * \brief Create a new instance of Path
 **/
int path_new(lua_State* L);

/**
 * \brief Free an instance of Path
 */
int path_free(lua_State* L);

/**
 * \brief Retrieve an instance of Path from the lua stack
 */
PathElements* path_get(lua_State* L, int index);

}
