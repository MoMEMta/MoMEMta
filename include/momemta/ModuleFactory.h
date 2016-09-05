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
#include <unordered_map>
#include <vector>

#include <momemta/PluginFactory.h>

// Forward declaration
class Module;
class ParameterSet;
class Pool;

// Register ModuleFactory used by all the modules
using ModuleFactory = PluginFactory<Module* (std::shared_ptr<Pool>, const ParameterSet&)>;

#define REGISTER_MODULE(type) \
    static const ModuleFactory::PMaker<type> PLUGIN_UNIQUE_NAME(s_module , __LINE__)(#type)

#define REGISTER_MODULE_NAME(name, type) \
    static const ModuleFactory::PMaker<type> PLUGIN_UNIQUE_NAME(s_module , __LINE__)(name)
