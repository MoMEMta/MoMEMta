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

const std::vector<Configuration::Module>& Configuration::getModules() const {
    return modules;
}

const ParameterSet& Configuration::getCubaConfiguration() const {
    return cuba_configuration;
}

const ParameterSet& Configuration::getGlobalParameters() const {
    return global_parameters;
}

InputTag Configuration::getIntegrand() const {
    return integrand;
}

std::vector<PathElementsPtr> Configuration::getPaths() const {
    return paths;
}

Configuration Configuration::freeze() const {
    Configuration c = *this;

    c.global_parameters.freeze();
    c.cuba_configuration.freeze();
    for (auto& module: c.modules) {
        module.parameters.freeze();
        // Attach global configuration to each module
        module.parameters.setGlobalParameters(c.global_parameters);
    }

    return c;
}
