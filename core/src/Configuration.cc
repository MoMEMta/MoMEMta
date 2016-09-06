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

#include <momemta/Configuration.h>
#include <momemta/ConfigurationReader.h>
#include <momemta/Path.h>
#include <momemta/ParameterSet.h>

Configuration::Module::Module(const Configuration::Module& other) {
    name = other.name;
    type = other.type;

    if (other.parameters)
        parameters.reset(other.parameters->clone());
}

Configuration::Module::Module(const Configuration::Module&& other) {
    name = std::move(other.name);
    type = std::move(other.type);
    parameters = std::move(other.parameters);
}

Configuration::Module& Configuration::Module::operator=(Configuration::Module other) {
    std::swap(name, other.name);
    std::swap(type, other.type);
    std::swap(parameters, other.parameters);

    return *this;
}

Configuration::Configuration(const Configuration& other) {
    modules = other.modules;
    if (other.global_parameters)
        global_parameters.reset(other.global_parameters->clone());
    if (other.cuba_configuration)
        cuba_configuration.reset(other.cuba_configuration->clone());
    integrands = other.integrands;
    paths = other.paths;
    n_dimensions = other.n_dimensions;
}

Configuration::Configuration(const Configuration&& other) {
    modules = std::move(other.modules);
    global_parameters = std::move(other.global_parameters);
    cuba_configuration = std::move(other.cuba_configuration);
    integrands = std::move(other.integrands);
    paths = std::move(other.paths);
    n_dimensions = other.n_dimensions;
}

Configuration& Configuration::operator=(Configuration other) {
    std::swap(*this, other);
    return *this;
}

const std::vector<Configuration::Module>& Configuration::getModules() const {
    return modules;
}

const ParameterSet& Configuration::getCubaConfiguration() const {
    return *cuba_configuration;
}

const ParameterSet& Configuration::getGlobalParameters() const {
    return *global_parameters;
}

std::vector<InputTag> Configuration::getIntegrands() const {
    return integrands;
}

std::vector<PathElementsPtr> Configuration::getPaths() const {
    return paths;
}

std::size_t Configuration::getNDimensions() const {
    return n_dimensions;
}

Configuration Configuration::freeze() const {
    Configuration c = *this;

    c.global_parameters->freeze();
    c.cuba_configuration->freeze();
    for (auto& module: c.modules) {
        module.parameters->freeze();
        // Attach global configuration to each module
        module.parameters->setGlobalParameters(*c.global_parameters);
    }

    return c;
}
