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


#include <momemta/ConfigurationSet.h>
#include <momemta/Utils.h>

#include <logging.h>
#include <lua/utils.h>

ConfigurationSet::ConfigurationSet(const std::string& module_type, const std::string& module_name) {
    m_set.emplace("@type", Element(module_type));
    m_set.emplace("@name", Element(module_name));
}

void ConfigurationSet::parse(lua_State* L, int index) {

    TRACE("[parse] >> stack size = {}", lua_gettop(L));
    size_t absolute_index = lua::get_index(L, index);

    lua_pushnil(L);
    while (lua_next(L, absolute_index) != 0) {

        std::string key = lua_tostring(L, -2);

        TRACE("[parse] >> key = {}", key);

        try {
            boost::any value;
            bool lazy = false;
            std::tie(value, lazy) = parseItem(key, L, -1);

            m_set.emplace(key, Element(value, lazy));
        } catch(...) {
            LOG(emerg) << "Exception while trying to parse parameter " << getModuleType() << "." << getModuleName() << "::" << key;
            lua_pop(L, 1);
            std::rethrow_exception(std::current_exception());
        }

        lua_pop(L, 1);
    }

    TRACE("[parse] << stack size = {}", lua_gettop(L));
}

std::pair<boost::any, bool> ConfigurationSet::parseItem(const std::string& key, lua_State* L, int index) {
    UNUSED(key);

    return lua::to_any(L, index);
}

bool ConfigurationSet::exists(const std::string& name) const {
    auto value = m_set.find(name);
    return (value != m_set.end());
}

void ConfigurationSet::setInternal(const std::string& name, Element& element, const boost::any& value) {
    UNUSED(name);

    element.value = value;
    element.lazy = false;
}

void ConfigurationSet::freeze() {
    if (frozen)
        return;

    frozen = true;

    for (auto& p: m_set) {
        auto& element = p.second;
        try {
            if (element.lazy) {
                element.lazy = false;
                if (element.value.type() == typeid(lua::LazyFunction)) {
                    element.value = boost::any_cast<lua::LazyFunction>(element.value)();
                } else if (element.value.type() == typeid(lua::LazyTableField)) {
                    element.value = boost::any_cast<lua::LazyTableField>(element.value)();
                }
            } else {
                // Recursion
                if (element.value.type() == typeid(ConfigurationSet)) {
                    ConfigurationSet& s = boost::any_cast<ConfigurationSet&>(element.value);
                    s.freeze();
                } else if (element.value.type() == typeid(std::vector<ConfigurationSet>)) {
                    std::vector<ConfigurationSet>& v = boost::any_cast<std::vector<ConfigurationSet>&>(element.value);
                    for (auto& c: v)
                        c.freeze();
                }
            }
        } catch(...) {
            LOG(emerg) << "Exception while trying to parse parameter " << getModuleType() << "." << getModuleName() << "::" << p.first;
            std::rethrow_exception(std::current_exception());
        }
    }
}

void ConfigurationSet::setGlobalConfiguration(const ConfigurationSet& configuration) {
    m_set.emplace("@global_configuration", Element(configuration, false));
}

// ---------

LazyConfigurationSet::LazyConfigurationSet(const std::string& name):
    ConfigurationSet("table", name) { }

std::pair<boost::any, bool> LazyConfigurationSet::parseItem(const std::string& key, lua_State* L, int index) {
    UNUSED(index);

    return std::make_pair(lua::LazyTableField(L, getModuleName(), key), true);
}

void LazyConfigurationSet::setInternal(const std::string& name, Element& element, const boost::any& value) {

    // We know that this set is not frozen, so *all* the items in the map
    // are actually lazy reference to lua table values.
    // Instead of editing directly the value, we edit the value of the global table
    // directly in lua user-space
    UNUSED(name);

    assert(element.lazy);
    assert(element.value.type() == typeid(lua::LazyTableField));

    boost::any_cast<lua::LazyTableField&>(element.value).set(value);
}
