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

#include <momemta/Pool.h>

#include <momemta/Logging.h>

void Pool::remove(const InputTag& tag, bool force/* = true*/) {
    auto it = m_storage.find(tag);
    if (it == m_storage.end())
        return;

    if (!force && it->second.valid)
        return;

    m_storage.erase(it);
}

void Pool::remove_if_invalid(const InputTag& tag) {
    remove(tag, false);
}

boost::any Pool::reserve(const InputTag& tag) {
    auto it = m_storage.find(tag);
    if (it == m_storage.end()) {
        // This tag do not exist currently in the pool
        // Reserve a slot, but mark it as invalid.
        // Once a module inform the pool it produces such a tag, the slot will
        // be flagged as valid.
        PoolContent content { boost::any(), false };
        it = m_storage.emplace(tag, content).first;
    }

    // Update current module description
    Description& description = get_description();
    description.inputs.push_back(tag);

    return it->second.ptr;
}

void Pool::alias(const InputTag& from, const InputTag& to) {
    if (from.isIndexed() || to.isIndexed()) {
        throw std::invalid_argument("Indexed input tag cannot be passed as argument of the pool. Use the `get` function of the input tag to retrieve its content.");
    }

    auto from_it = m_storage.find(from);
    if (from_it == m_storage.end())
        throw tag_not_found_error("No such tag in pool: " + from.toString());

    auto to_it = m_storage.find(to);
    if (to_it != m_storage.end())
        throw duplicated_tag_error("A module already produced the tag '" + to.toString() + "'");

    m_storage[to] = m_storage[from];
}

bool Pool::exists(const InputTag& tag) const {
    auto it = m_storage.find(tag);
    return it != m_storage.end();
}

void Pool::current_module(const Configuration::Module& module) {
    m_current_module = module;
}

void Pool::current_module(const std::string& name) {
    Configuration::Module module;
    module.name = name;
    module.type = "@" + name;

    m_current_module = module;
}

class invalid_state: public std::runtime_error {
    using std::runtime_error::runtime_error;
};

void Pool::freeze() {
    m_frozen = true;

    m_current_module.name.clear();

    // Iterate over the storage, and check if any block is invalid.
    for (const auto& it: m_storage) {
        if (!it.second.valid) {
            LOG(fatal) << "Memory block '" << it.first.toString() << "' is flagged as invalid. This should not happen. Please open a bug report at <>.";
            throw invalid_state("Memory pool state is invalid");
        }
    }
}

Description& Pool::get_description() const {
    assert(! m_current_module.name.empty());

    auto it = m_description.find(m_current_module.name);
    if (it == m_description.end()) {
        Description description;
        description.module = m_current_module;
        it = m_description.emplace(std::make_pair(m_current_module.name, description)).first;
    }

    return it->second;
}