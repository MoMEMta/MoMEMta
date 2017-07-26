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


#include <lua.hpp>

#include <momemta/any.h>
#include <momemta/Logging.h>
#include <momemta/ConfigurationReader.h>
#include <momemta/ModuleFactory.h>
#include <momemta/ParameterSet.h>

#include <ExecutionPath.h>
#include <lua/LazyTable.h>
#include <lua/ParameterSetParser.h>
#include <lua/utils.h>
#include <strings/StringPiece.h>

ConfigurationReader::ConfigurationReader(const std::string& file) :
        ConfigurationReader(file, ParameterSet()) {
    // Empty
}

ConfigurationReader::ConfigurationReader(const std::string& from, const ParameterSet& parameters) {

    lua_state = lua::init_runtime(this);

    lua::inject_parameters(lua_state.get(), parameters);

    momemta::StringPiece from_view = from;
    bool from_string = from_view.Consume("!");

    if (from_string) {
        LOG(debug) << "Parsing LUA configuration from string";

        // Parse string
        if (luaL_dostring(lua_state.get(), from_view.data())) {
            std::string error = lua_tostring(lua_state.get(), -1);
            LOG(fatal) << "Failed to parse configuration string: " << error;
            throw lua::invalid_configuration_file(error);
        }

    } else {
        LOG(debug) << "Parsing LUA configuration from " << from;

        // Parse file
        if (luaL_dofile(lua_state.get(), from.c_str())) {
            std::string error = lua_tostring(lua_state.get(), -1);
            LOG(fatal) << "Failed to parse configuration file: " << error;
            throw lua::invalid_configuration_file(error);
        }
    }

    // FIXME: Find a better way of doing that

    // Read global parameters from global variable named 'parameters'
    configuration.global_parameters.reset(new lua::LazyTable(lua_state, "parameters"));
    int type = lua_getglobal(lua_state.get(), "parameters");
    if (type == LUA_TTABLE) {
        LOG(debug) << "Parsing global parameters.";
        ParameterSetParser::parse(*configuration.global_parameters, lua_state.get(), -1);
    }
    lua_pop(lua_state.get(), 1);

    // Read cuba configuration
    configuration.cuba_configuration.reset(new lua::LazyTable(lua_state, "cuba"));
    type = lua_getglobal(lua_state.get(), "cuba");
    if (type == LUA_TTABLE) {
        LOG(debug) << "Parsing cuba configuration.";
        ParameterSetParser::parse(*configuration.cuba_configuration, lua_state.get(), -1);
    }
    lua_pop(lua_state.get(), 1);

    for (auto& m: configuration.modules) {
        LOG(debug) << "Module declared: " << m.type << "::" << m.name;

        lua_getglobal(lua_state.get(), m.type.c_str());
        lua_getfield(lua_state.get(), -1, m.name.c_str());

        m.parameters.reset(new ParameterSet());
        ParameterSetParser::parse(*m.parameters, lua_state.get(), -1);

        lua_pop(lua_state.get(), 2);
    }
}

void ConfigurationReader::onModuleDeclared(const std::string& type, const std::string& name) {
    Configuration::ModuleDecl module;
    module.name = name;
    module.type = type;

    configuration.modules.push_back(module);
}

void ConfigurationReader::onIntegrandDeclared(const InputTag& tag) {
    configuration.integrands.push_back(tag);
}

void ConfigurationReader::onNewPath(const ExecutionPath& path) {
    configuration.paths.push_back(std::make_shared<ExecutionPath>(path));
}

void ConfigurationReader::addIntegrationDimension() {
    configuration.n_dimensions++;
}

void ConfigurationReader::onNewInputDeclared(const std::string& name) {
    configuration.inputs.push_back(name);
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
