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

#include <lua.hpp>

#include <momemta/Logging.h>
#include <momemta/ConfigurationReader.h>
#include <momemta/ModuleFactory.h>
#include <momemta/ParameterSet.h>
#include <momemta/Path.h>

#include <lua/utils.h>

ConfigurationReader::ConfigurationReader(const std::string& file) {

    LOG(debug) << "Parsing LUA configuration from " << file;
    lua_state = lua::init_runtime(this);

    // Parse file
    if (luaL_dofile(lua_state.get(), file.c_str())) {
        std::string error = lua_tostring(lua_state.get(), -1);
        LOG(fatal) << "Failed to parse configuration file: " << error;
        throw lua::invalid_configuration_file(error);
    }

    // FIXME: Find a better way of doing that

    // Read global parameters from global variable named 'parameters'
    configuration.global_parameters.reset(new LazyParameterSet(lua_state, "parameters"));
    int type = lua_getglobal(lua_state.get(), "parameters");
    if (type == LUA_TTABLE) {
        LOG(debug) << "Parsing global parameters.";
        configuration.global_parameters->parse(lua_state.get(), -1);
    }
    lua_pop(lua_state.get(), 1);

    // Read cuba configuration
    configuration.cuba_configuration.reset(new LazyParameterSet(lua_state, "cuba"));
    type = lua_getglobal(lua_state.get(), "cuba");
    if (type == LUA_TTABLE) {
        LOG(debug) << "Parsing cuba configuration.";
        configuration.cuba_configuration->parse(lua_state.get(), -1);
    }
    lua_pop(lua_state.get(), 1);

    for (auto& m: configuration.modules) {
        LOG(debug) << "Configuration declared module " << m.type << "::" << m.name;

        lua_getglobal(lua_state.get(), m.type.c_str());
        lua_getfield(lua_state.get(), -1, m.name.c_str());

        m.parameters.reset(new ParameterSet(m.type, m.name));
        m.parameters->parse(lua_state.get(), -1);

        lua_pop(lua_state.get(), 2);
    }
}

void ConfigurationReader::onModuleDeclared(const std::string& type, const std::string& name) {
    Configuration::Module module;
    module.name = name;
    module.type = type;

    configuration.modules.push_back(module);
}

void ConfigurationReader::onIntegrandDeclared(const InputTag& tag) {
    configuration.integrands.push_back(tag);
}

void ConfigurationReader::onNewPath(PathElementsPtr path) {
    configuration.paths.push_back(path);
}

void ConfigurationReader::addIntegrationDimension() {
    configuration.n_dimensions++;
}

ParameterSet& ConfigurationReader::getGlobalParameters() {
    return *configuration.global_parameters;
}

ParameterSet& ConfigurationReader::getCubaConfiguration() {
    return *configuration.cuba_configuration;
}

Configuration ConfigurationReader::freeze() const {
    return configuration.freeze();
}
