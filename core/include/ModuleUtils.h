/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2017  Universite catholique de Louvain (UCL), Belgium
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

#include <momemta/ModuleRegistry.h>
#include <momemta/ParameterSet.h>

#include <lib/optional.h>

namespace momemta {

/**
 * \brief Validates parameters against a module definition
 *
 * \param module_def Definition of the module
 * \param parameters Parameters to validate
 * \return True if the parameters match the definition, false otherwise
 */
bool validateModuleParameters(const ModuleList::value_type& module_def, const ParameterSet& parameters);

/**
 * Return the list of InputTag associated with a given input, if it exists
 *
 * \param input The input definition
 * \param parameters Modules' parameters where to look for the InputTag
 * \return The list of InputTag associated with \p input, if found in \p parameters.
 */
gtl::optional<std::vector<InputTag>> getInputTagsForInput(const ArgDef& input, const ParameterSet& parameters);

}