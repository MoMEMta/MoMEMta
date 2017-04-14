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

namespace momemta {

/// Defines an attribute
struct AttrDef {
    std::string name;
    std::string type;
    std::string default_value;
    /**
     * If true, this attribute is global and must be declared in the global `parameters` table. Such an attribute
     * must be ignored when doing validation, as the user is not supposed to define it.
     */
    bool global;
    bool optional;
};

/// Defines an input / output
struct ArgDef {
    std::string name;
    std::string default_value; ///< Only meaningful for inputs
    bool optional; ///< Only meaningful for inputs
    bool many; ///< Only meaningful for inputs. If True, this input points to a list instead of a single value

    /**
     * If not empty, this input is nested inside attributes. The order in this list is the nesting order
     *
     * Example:
     *   Suppose an input 'attr/nested_attr/input=module::output'.
     *   `nested_attributes` will be filled with definitions of `attr` and `nested_attr`, in this order.
     *
     * \note Only meaningful for inputs
     */
    std::vector<AttrDef> nested_attributes;
};

/**
 * Defines a module, listing its attributes, inputs and outputs
 */
struct ModuleDef {
    std::vector<AttrDef> attributes;
    std::vector<ArgDef> inputs;
    std::vector<ArgDef> outputs;

    std::string name;

    /// If true, this module is internal and not associated to a real implementation
    bool internal = false;

    /// A sticky module is a module which can't be removed from the graph, even if it's output is not used
    bool sticky = false;
};

using ModuleList = std::vector<ModuleDef>;

}