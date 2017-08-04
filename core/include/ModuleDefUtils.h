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

#include <string>

#include <momemta/ModuleDef.h>

#include <lib/optional.h>

namespace momemta {

/**
 * Check if an input or an attribute \p name exists in the definition \p def
 * \param name The name to look for
 * \param def The definition of the module
 * \return True if such input or attribute exist, false otherwise
 */
bool inputOrAttrExists(const std::string& name, const ModuleDef& def);

gtl::optional<AttrDef> findAttr(const std::string& name, const ModuleDef& def);
}