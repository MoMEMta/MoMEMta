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

#include <momemta/ParameterSet.h>

#include <momemta/Unused.h>

#include <lua/LazyTable.h>
#include <lua/utils.h>

ParameterSet::ParameterSet(const std::string& module_type, const std::string& module_name) {
    m_set.emplace("@type", Element(module_type));
    m_set.emplace("@name", Element(module_name));
}

const momemta::any& ParameterSet::rawGet(const std::string& name) const {
    auto value = m_set.find(name);
    if (value == m_set.end())
        throw not_found_error("Parameter '" + name + "' not found.");

    return value->second.value;
}

bool ParameterSet::lazy() const {
    return false;
}

bool ParameterSet::exists(const std::string& name) const {
    auto value = m_set.find(name);
    return (value != m_set.end());
}

void ParameterSet::create(const std::string& name, const momemta::any& value) {
    m_set.emplace(name, Element(value, false));
}

void ParameterSet::setInternal(const std::string& name, Element& element, const momemta::any& value) {
    UNUSED(name);

    element.value = value;
    element.lazy = false;
}

void ParameterSet::freeze() {
    if (frozen)
        return;

    frozen = true;

    for (auto& p: m_set) {
        auto& element = p.second;
        try {
            if (element.lazy) {
                element.lazy = false;
                if (element.value.type() == typeid(lua::LazyFunction)) {
                    element.value = momemta::any_cast<lua::LazyFunction>(element.value)();
                } else if (element.value.type() == typeid(lua::LazyTableField)) {
                    element.value = momemta::any_cast<lua::LazyTableField>(element.value)();
                }
            } else {
                // Recursion
                if (element.value.type() == typeid(ParameterSet)) {
                    ParameterSet& s = momemta::any_cast<ParameterSet&>(element.value);
                    s.freeze();
                } else if (element.value.type() == typeid(std::vector<ParameterSet>)) {
                    std::vector<ParameterSet>& v = momemta::any_cast<std::vector<ParameterSet>&>(element.value);
                    for (auto& c: v)
                        c.freeze();
                }
            }
        } catch(...) {
            LOG(fatal) << "Exception while trying to parse parameter " << getModuleType() << "." << getModuleName() << "::" << p.first;
            std::rethrow_exception(std::current_exception());
        }
    }
}

void ParameterSet::setGlobalParameters(const ParameterSet& parameters) {
    m_set.emplace("@global_parameters", Element(parameters, false));
}

ParameterSet* ParameterSet::clone() const {
    return new ParameterSet(*this);
}

std::vector<std::string> ParameterSet::getNames() const {
    std::vector<std::string> names;
    for (const auto& it: m_set) {
        names.push_back(it.first);
    }

    return names;
}