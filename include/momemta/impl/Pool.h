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

#include <momemta/Pool.h>

#include <assert.h>
#include <memory>
#include <unordered_map>

#include <boost/any.hpp>

#include <momemta/Value.h>
#include <momemta/impl/ValueProxy.h>

// A simple memory pool

template <typename T> Value<T> Pool::get(const InputTag& tag) const {

    auto it = m_storage.find(tag);
    if (it == m_storage.end()) {
        if (tag.isIndexed()) {
            it = create<std::vector<T>>(tag, false);
        } else {
            it = create<T>(tag, false);
        }
    }

    Value<T> value(ValueProxy<const T>::create(this, tag));

    if (!m_frozen) {
        // Update current module description
        Description& description = get_description();
        description.inputs.push_back(tag);
    }

    return value;
}

template <typename T> std::shared_ptr<const T> Pool::raw_get(const InputTag& tag) const {

    auto it = m_storage.find(tag);
    if (it == m_storage.end())
        throw tag_not_found_error("No such tag in pool: " + tag.toString());

    PoolContent& v = it->second;
    std::shared_ptr<T>& ptr = boost::any_cast<std::shared_ptr<T>&>(v.ptr);

    return std::const_pointer_cast<const T>(ptr);
}

template <typename T, typename... Args> std::shared_ptr<T> Pool::put(const InputTag& tag, Args... args) {
    auto it = m_storage.find(tag);
    if (it != m_storage.end()) {
        if (it->second.valid)
            throw duplicated_tag_error("A module already produced the tag '" + tag.toString() + "'");
        // A module already requested this block in read-mode. This will only work if the block does not require a non-trivial constructor:
        if (sizeof...(Args))
            throw constructor_tag_error("A module already requested the tag '" + tag.toString()
                                                + "' which seems to require a constructor call. This is currently not supported.");
        // Since the memory is allocated, simply consider the block as valid.
        it->second.valid = true;

        // If the block is empty, it's a delayed instantiation. Simply flag the block as valid, and allocate memory for it
        if (it->second.ptr.empty()) {
            std::shared_ptr<T> ptr(new T(std::forward<Args>(args)...));
            it->second.ptr = boost::any(ptr);
        }

    } else {
        it = create<T, Args ...>(tag, true, std::forward<Args>(args)...);
    }

    // Update current module description
    Description& description = get_description();
    description.outputs.push_back(tag.parameter);

    return boost::any_cast<std::shared_ptr<T>>(it->second.ptr);
}

template <typename T, typename... Args> Pool::PoolStorage::iterator Pool::create(
        const InputTag& tag, bool valid/* = true*/, Args... args) const {

    std::shared_ptr<T> ptr = std::make_shared<T>(std::forward<Args>(args)...);
    PoolContent content = {boost::any(ptr), valid};

    return m_storage.emplace(tag, content).first;
}
