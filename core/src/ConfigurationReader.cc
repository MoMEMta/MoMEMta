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


#include <boost/any.hpp>
#include <logging.h>
#include <lua.hpp>

#include <momemta/ConfigurationSet.h>
#include <momemta/ConfigurationReader.h>
#include <momemta/ModuleFactory.h>

#include <lua/utils.h>

ConfigurationReader::ConfigurationReader(const std::string& file) {

    LOG(debug) << "Parsing LUA configuration from " << file;
    lua_state = luaL_newstate();

    luaL_openlibs(lua_state);

    // Register function load_modules
    lua::setup_hooks(lua_state, this);

    // Register existing modules
    lua::register_modules(lua_state, this);

    // Parse file
    if (luaL_dofile(lua_state, file.c_str())) {
        std::string error = lua_tostring(lua_state, -1);
        LOGGER->critical("Failed to parse configuration file: {}", error);
        throw lua::invalid_configuration_file(error);
    }

    // FIXME: Find a better way of doing that

    // Read global configuration from global variable named 'configuration'
    m_global_configuration.reset(new ConfigurationSet("configuration", "configuration"));
    int type = lua_getglobal(lua_state, "configuration");
    if (type == LUA_TTABLE) {
        LOG(debug) << "Parsing global configuration.";
        m_global_configuration->parse(lua_state, -1);
    }
    lua_pop(lua_state, 1);

    // Read vegas configuration
    m_vegas_configuration.reset(new ConfigurationSet("vegas", "vegas"));
    type = lua_getglobal(lua_state, "vegas");
    if (type == LUA_TTABLE) {
        LOG(debug) << "Parsing vegas configuration.";
        m_vegas_configuration->parse(lua_state, -1);
    }
    lua_pop(lua_state, 1);

    for (auto& m: m_modules) {
        LOG(debug) << "Configuration declared module " << m.type << "::" << m.name;

        lua_getglobal(lua_state, m.type.c_str());
        lua_getfield(lua_state, -1, m.name.c_str());

        m.parameters = ConfigurationSet(m.type, m.name, m_global_configuration);
        m.parameters.parse(lua_state, -1);

        lua_pop(lua_state, 2);
    }
}

ConfigurationReader::~ConfigurationReader() {
    lua_close(lua_state);
    lua_state = nullptr;
}

void ConfigurationReader::addModule(const std::string& type, const std::string& name) {
    Configuration::Module module;
    module.name = name;
    module.type = type;
    m_modules.push_back(module);
}

Configuration ConfigurationReader::freeze() const {
    return Configuration(*this);
}
