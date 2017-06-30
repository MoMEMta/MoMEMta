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

#include <momemta/ModuleDef.h>

// Forward declaration
class Module;
class ParameterSet;
class Pool;

namespace momemta {

/**
 * A container for module registration data
 */
struct ModuleRegistrationData {
    ModuleRegistrationData() {}
    ModuleRegistrationData(const ModuleDef& def): module_def(def) { }

    /// Definition of the module (inputs, outputs, attributes, ...)
    ModuleDef module_def;
};

namespace registration {

// Builder class passed to the REGISTER_MODULE macro
class ModuleDefBuilder {
public:
    explicit ModuleDefBuilder(const std::string& name);

    template <typename ModuleType>
    ModuleDefBuilder& Type() {
        // Currently not used
        return *this;
    }

    /**
     \brief Adds an attribute to the module definition (and returns `*this`).

     The \p spec has format `<name>:<type>` or `<name>:<type>=<default>`

     where `<name>` matches regexp `[a-zA-Z][a-zA-Z0-9_]*`

     `<type>` can be:
       - `string`, `int`, `double`, `bool`, `pset`, `path`
       - `list(string)`, `list(double)`, ...(meaning lists of the above types)

     `<default>`, if included, is the default value of the attribute.
     For lists use `[a, b, c]` format.
    */
    ModuleDefBuilder& Attr(const std::string& spec);

    /**
     * \brief Adds a global attribute to the module definition (and returns `*this`).
     *
     * \details \copydetails Attr()
     */
    ModuleDefBuilder& GlobalAttr(const std::string& spec);

    /**
     * \brief Adds an optional attribute to the module definition (and returns `*this`).
     *
     * \details \copydetails Attr()
     */
    ModuleDefBuilder& OptionalAttr(const std::string& spec);

    /**
     \brief Adds an input to the module definition (and returns `*this`).

     The \p spec has format `<name>` or `<name>=<default>`

     where `<name>` matches regexp `[a-zA-Z][a-zA-Z0-9_]*`

     Input can also be nested inside attributes. In this case, \p spec has format
     `<attr>/[<attr>/]*<name>` where `<attr>` matches regex defined in Attr() and `<name>`
     matches regexp defined above.
    */
    /* TODO: Add support for type when defining inputs, in the same way as attributes
       TODO: we can then ensure that inputs are connected to compatible outputs */
    ModuleDefBuilder& Input(const std::string& spec);

    /**
     * \brief Adds an optional input to the module definition (and returns `*this`).
     *
     * \details \copydetails Input()
     */
    ModuleDefBuilder& OptionalInput(const std::string& spec);

    /**
     * \brief Adds inputs to the module definition (and returns `*this`).
     *
     * \details \copydetails Input()
     */
    ModuleDefBuilder& Inputs(const std::string& spec);

    /**
     * \brief Adds optional inputs to the module definition (and returns `*this`).
     *
     * \details \copydetails Input()
     */
    ModuleDefBuilder& OptionalInputs(const std::string& spec);

    ModuleDefBuilder& Output(const std::string& spec);

    /**
     * \brief Flag this module as sticky
     *
     * A sticky module is a module which can't be removed from the graph, even if its outputs are not used
     */
    ModuleDefBuilder& Sticky();

    ModuleRegistrationData Build() const;

    std::string name() const;

private:
    mutable ModuleRegistrationData reg_data;

    std::vector<std::string> attrs;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
};

}

}