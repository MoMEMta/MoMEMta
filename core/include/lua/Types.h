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

#include <boost/any.hpp>

struct lua_State;

/**
 * \file
 * \brief Generic functions to deal with custom lua types
 */


/** \brief Push a new metatable on the stack
 *
 * The metatable contains one additional field, `__type`,
 * describing of which type is the table associated to this
 * metatable
 *
 * \param L the lua state
 * \param name the type's name
 */
void push_type_metatable(lua_State* L, const char* name);

/**
 * \brief Get the type of a custom table
 *
 * \param L the lua state
 * \param index the index on the stack of the custom table
 *
 * \return The type's name
 */
std::string get_custom_type_name(lua_State* L, int index);

/**
 * \brief Convert a lua custom table to a boost::any value
 *
 * \param L the lua state
 * \param index the index on the stack of the custom table to convert
 *
 * \return a boost::any value encapsulating the custom type
 */
boost::any get_custom_type_ptr(lua_State* L, int index);
