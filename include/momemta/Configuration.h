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

#include <momemta/ConfigurationSet.h>

class ConfigurationReader;

/**
 * \brief A frozen snapshot of the configuration file.
 *
 * All the parameters are enforced to be static (ie, all lua::Lazy parameters are evaluated)
 */
class Configuration {
    public:
        /**
         * \brief A module defined from the configuration file
         *
         * Stores the name, type and parameters of the module
         */
        struct Module {
            std::string name; ///< Name of the module (user-defined from the configuration file)
            std::string type; ///< Type of the module (mapped one-to-one with a C++ class inherting from Module)
            ConfigurationSet parameters; ///< Module's parameters, as parsed from the configuration file
        };

        /// \return The list of modules declared from the configuration file
        const std::vector<Module>& getModules() const;
        /// \return The cuba configuration as declared in the configuration file
        const ConfigurationSet& getCubaConfiguration() const;
        /// \return The global configuration as declared in the configuration file
        const ConfigurationSet& getGlobalConfiguration() const;

    private:
        friend class ConfigurationReader;
        Configuration(const ConfigurationReader&);

        std::vector<Module> modules;
        ConfigurationSet global_configuration;
        ConfigurationSet cuba_configuration;
};
