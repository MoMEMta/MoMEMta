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

#include <memory>
#include <string>
#include <vector>

#include <momemta/Configuration.h>
#include <momemta/ILuaCallback.h>

class ParameterSet;
struct PathElements;
struct lua_State;

/**
 * \brief A lua configuration file parser
 *
 * This class handles the parsing of the lua configuration file. The configuration files describes which modules should be run in order to form an integrand. This integrand is then integrated using CUBA and MoMEMta and the result of the integral is returned.
 *
 * A module is a single element of a more complex scheme. Itself, it only performs a simple tasks (flattening a Breit-Wigner for example), but combined with other modules, it's possible to define complex integrand.
 *
 * A module can be seen like a box with a fixed number of inputs and outputs. The configuration file is used to specify a list of module to run, and to connect outputs of modules to inputs of other modules.
 *
 * \todo Discuss the `parameters` table, `cuba` table and the concept of freezing and delayed execution
 *
 */
class ConfigurationReader: public ILuaCallback {
    public:
        ConfigurationReader(const std::string&);

        virtual void onModuleDeclared(const std::string& type, const std::string& name) override;
        virtual void onIntegrandDeclared(const InputTag& tag) override;
        virtual void onNewPath(PathElements* path) override;
        virtual void addIntegrationDimension() override;

        ParameterSet& getGlobalParameters();
        ParameterSet& getCubaConfiguration();

        /**
         * \brief Freeze the configuration
         *
         * \return A freezed copy of the configuration
         */
        Configuration freeze() const;

    private:
        friend class Configuration;

        Configuration configuration;

        std::shared_ptr<lua_State> lua_state;
};
