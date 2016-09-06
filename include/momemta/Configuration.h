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

#include <momemta/impl/InputTag_fwd.h>

class ConfigurationReader;
class ParameterSet;
struct PathElements;

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
            std::string type; ///< Type of the module (mapped one-to-one with a C++ class inherting from Module). If
                              ///< starting with '@', this module is virtual.
            std::shared_ptr<ParameterSet> parameters; ///< Module's parameters, as parsed from the configuration file

            Module() = default;
            Module(const Module&);
            Module(const Module&&);
            Module& operator=(Module);
        };

        /// \return The list of modules declared from the configuration file
        const std::vector<Module>& getModules() const;
        /// \return The cuba configuration as declared in the configuration file
        const ParameterSet& getCubaConfiguration() const;
        /// \return The global parameters as declared in the configuration file
        const ParameterSet& getGlobalParameters() const;
        /// \return The integrand InputTag as declared in the configuration
        std::vector<InputTag> getIntegrands() const;
        
        /// \return The list of Paths as declared in the configuration
        std::vector<PathElements*> getPaths() const;

        /// \return The number of integration dimensions required in the configuration
        size_t getNDimensions() const;

        /**
         * \brief Copy constructor.
         *
         * The current implementation *clones* the ParameterSet members.
         */
        Configuration(const Configuration&);

        /**
         * \brief Move constructor.
         */
        Configuration(const Configuration&&);

        Configuration& operator=(Configuration);

    private:
        friend class ConfigurationReader;
        Configuration(): n_dimensions(0) {};

        /// Return a frozen copy of this configuration
        Configuration freeze() const;

        std::vector<Module> modules;
        std::shared_ptr<ParameterSet> global_parameters;
        std::shared_ptr<ParameterSet> cuba_configuration;
        std::vector<InputTag> integrands;
        std::vector<PathElements*> paths;
        std::size_t n_dimensions;
};
