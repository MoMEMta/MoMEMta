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

#include <ModuleUtils.h>

#include <momemta/Logging.h>

#include <ModuleDefUtils.h>

namespace {
const ParameterSet *findPSet(const momemta::ArgDef& input_def, const ParameterSet& parameters) {
    const ParameterSet *pset = &parameters;
    std::vector<momemta::AttrDef> nested_attributes = input_def.nested_attributes;
    while (!nested_attributes.empty()) {
        momemta::AttrDef nested_attribute = nested_attributes.front();
        nested_attributes.erase(nested_attributes.begin());

        if (pset->existsAs<ParameterSet>(nested_attribute.name))
            pset = &pset->get<ParameterSet>(nested_attribute.name);
        else {
            pset = nullptr;
            break;
        }
    }

    return pset;
}
}

bool momemta::validateModuleParameters(const ModuleList::value_type& module_def, const ParameterSet& parameters) {

    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    // Check that all attributes are defined
    // TODO: We can imagine here validating also the type of the attribute
    for (const auto& attr_def: module_def.attributes) {

        // Ignore global or optional attributes
        if (attr_def.global || attr_def.optional)
            continue;

        if (! parameters.exists(attr_def.name))
            errors.emplace_back("Attribute not found: " + attr_def.name);
    }

    // Check that all inputs are defined
    for (const auto& input_def: module_def.inputs) {
        // Ignore optional inputs
        if (input_def.optional)
            continue;

        const ParameterSet* pset = &parameters;
        std::vector<AttrDef> nested_attributes = input_def.nested_attributes;
        while (!nested_attributes.empty()) {
            AttrDef nested_attribute = nested_attributes.front();
            nested_attributes.erase(nested_attributes.begin());

            if (pset->existsAs<ParameterSet>(nested_attribute.name))
                pset = &pset->get<ParameterSet>(nested_attribute.name);
            else {
                LOG(error) << "Attribute " << nested_attribute.name << " not found in PSet "
                           << pset->getModuleType() << "::" << pset->getModuleName();
                pset = nullptr;
                break;
            }
        }

        if (!pset || !pset->exists(input_def.name))
            errors.emplace_back("Input not found: " + input_def.name);
    }

    // Check for parameters not found in the module definition
    const auto& parameter_names = parameters.getNames();
    for (const auto& name: parameter_names) {

        // Ignore internal parameters
        if (name.length() > 0 && name[0] == '@')
            continue;

        if (! momemta::inputOrAttrExists(name, module_def))
            warnings.emplace_back("Unexpected parameter: " + name);
    }

    if (! warnings.empty()) {
        // Warnings found during validation
        LOG(warning) << "Warnings found during validation of parameters for module "
                     << parameters.getModuleType() << "::" << parameters.getModuleName();
        for (const auto& warning: warnings)
            LOG(warning) << "    " << warning;
        LOG(warning) << "These parameters will never be used by the module, check your configuration file.";
    }

    if (!errors.empty()) {
        // Validation failed. Print errors.
        LOG(error) << "Validation of parameters for module " << parameters.getModuleType() << "::"
                   << parameters.getModuleName() << " failed: ";
        for (const auto& error: errors)
            LOG(error) << "    " << error;

        LOG(error) << "Check your configuration file.";
    }

    return errors.empty();
}

momemta::gtl::optional<std::vector<InputTag>> momemta::getInputTagsForInput(const ArgDef& input,
                                                                            const ParameterSet& parameters) {

    const ParameterSet* pset = findPSet(input, parameters);
    assert(pset || input.optional);

    if (! pset)
        return gtl::nullopt;

    if (input.optional && !pset->exists(input.name))
        return gtl::nullopt;

    if (input.many) {
        return pset->get<std::vector<InputTag>>(input.name);
    } else {
        return gtl::make_optional<std::vector<InputTag>>({pset->get<InputTag>(input.name)});
    }
}