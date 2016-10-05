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

#include <lua/LazyTable.h>

#include <momemta/Logging.h>
#include <momemta/Unused.h>

#include <lua/utils.h>

namespace lua {

LazyTableField::LazyTableField(lua_State *L, const std::string& table_name, const std::string& key) :
        Lazy(L) {
    this->table_name = table_name;
    this->key = key;
}

void LazyTableField::ensure_created() {
    // Push the table on the stack. Stack +1
    int type = lua_getglobal(L, table_name.c_str());
    // Discard the result. Stack -1
    lua_pop(L, 1);

    // Already existing
    if (type != LUA_TNIL)
        return;

    // Create a new table. Stack +1
    lua_newtable(L);
    // Set it global. Stack -1
    lua_setglobal(L, table_name.c_str());
}

boost::any LazyTableField::operator()() const {
    LOG(trace) << "[LazyTableField::operator()] >> stack size = " << lua_gettop(L);

    // Push the table on the stack. Stack +1
    lua_getglobal(L, table_name.c_str());

    // Push the requested field from the table to the stack. Stack +1
    lua_getfield(L, -1, key.c_str());

    boost::any value;
    bool lazy = false;
    std::tie(value, lazy) = to_any(L, -1);
    assert(!lazy);

    // Pop the field and the table from the stack. Stack -2
    lua_pop(L, 2);

    LOG(trace) << "[LazyTableField::operator()] << stack size = " << lua_gettop(L);

    return value;
}

void LazyTableField::set(const boost::any& value) {
    LOG(trace) << "[LazyTableField::set] >> stack size = " << lua_gettop(L);

    // Push the table on the stack. Stack +1
    lua_getglobal(L, table_name.c_str());

    // Push the value to the stack. Stack +1
    lua::push_any(L, value);

    // Pop the requested field from the stack and assign it to the table. Stack -1
    lua_setfield(L, -2, key.c_str());

    // Pop the table from the stack. Stack -1
    lua_pop(L, 1);

    LOG(trace) << "[LazyTableField::set] << stack size = " << lua_gettop(L);
}

LazyTable::LazyTable(std::shared_ptr<lua_State> L, const std::string& name):
        ParameterSet("table", name) {
    m_lua_state = L;
}

bool LazyTable::lazy() const {
    return true;
}

void LazyTable::create(const std::string& name, const boost::any& value) {

    lua::LazyTableField lazyField = lua::LazyTableField(m_lua_state.get(), getModuleName(), name);
    lazyField.ensure_created();
    lazyField.set(value);

    m_set.emplace(name, Element(lazyField, true));
}

void LazyTable::setInternal(const std::string& name, Element& element, const boost::any& value) {

    // We know that this set is not frozen, so *all* the items in the map
    // are actually lazy reference to lua table values.
    // Instead of editing directly the value, we edit the value of the global table
    // directly in lua user-space
    UNUSED(name);

    assert(element.lazy);
    assert(element.value.type() == typeid(lua::LazyTableField));

    boost::any_cast<lua::LazyTableField&>(element.value).set(value);
}

void LazyTable::freeze() {
    ParameterSet::freeze();

    // Release lua_State. We don't need it anymore
    m_lua_state.reset();
}

LazyTable* LazyTable::clone() const {
    return new LazyTable(*this);
}


}