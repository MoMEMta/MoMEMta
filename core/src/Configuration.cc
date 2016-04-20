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

const std::vector<Configuration::Module>& Configuration::getModules() const {
    return modules;
}

const ConfigurationSet& Configuration::getVegasConfiguration() const {
    return vegas_configuration;
}

const ConfigurationSet& Configuration::getGlobalConfiguration() const {
    return global_configuration;
}

Configuration::Configuration(const ConfigurationReader& reader) {
    modules = reader.m_modules;
    global_configuration = *reader.m_global_configuration.get();
    vegas_configuration = *reader.m_vegas_configuration.get();

    global_configuration.freeze();
    vegas_configuration.freeze();
    for (auto& module: modules) {
        module.parameters.freeze();
    }
}
