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

#include <lua/ParameterSetParser.h>

#include <momemta/Logging.h>

#include <lua/LazyTable.h>
#include <lua/utils.h>

void ParameterSetParser::parse(ParameterSet& p, lua_State* L, int index) {

    LOG(trace) << "[parse] >> stack size = " << lua_gettop(L);
    size_t absolute_index = lua::get_index(L, index);

    lua_pushnil(L);
    while (lua_next(L, absolute_index) != 0) {

        std::string key = lua_tostring(L, -2);

        LOG(trace) << "[parse] >> key = " << key;

        try {
            boost::any value;
            bool lazy = p.lazy();

            if (lazy) {
                value = lua::LazyTableField(L, p.getModuleName(), key);
            } else {
                std::tie(value, lazy) = lua::to_any(L, -1);
            }

            p.m_set.emplace(key, ParameterSet::Element(value, lazy));
        } catch(...) {
            LOG(fatal) << "Exception while trying to parse parameter " << p.getModuleType() << "." << p.getModuleName() << "::" << key;
            lua_pop(L, 1);
            std::rethrow_exception(std::current_exception());
        }

        lua_pop(L, 1);
    }

    LOG(trace) << "[parse] << stack size = " << lua_gettop(L);
}