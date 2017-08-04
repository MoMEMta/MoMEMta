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

#include <ModuleDefUtils.h>

#include <algorithm>

#include <lib/optional.h>

namespace momemta {

bool inputOrAttrExists(const std::string& name, const ModuleDef& def) {

    bool exists = false;

    // Look first in inputs
    auto it = std::find_if(def.inputs.begin(), def.inputs.end(),
                           [&name](const ArgDef& def) {
                               return def.name == name;
                           });

    exists |= it != def.inputs.end();

    if (! exists) {
        // Then in attributes

        auto it = std::find_if(def.attributes.begin(), def.attributes.end(),
                               [&name](const AttrDef& def) {
                                   return !def.global && def.name == name;
                               });

        exists |= it != def.attributes.end();
    }

    return exists;
}

gtl::optional<AttrDef> findAttr(const std::string& name, const ModuleDef& def) {
    auto it = std::find_if(def.attributes.begin(), def.attributes.end(),
        [&name](const AttrDef& def) {
            return !def.global && def.name == name;
        }
    );

    if (it == def.attributes.end())
        return gtl::nullopt;
    else
        return *it;
}

}