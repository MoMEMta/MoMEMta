/*
 *  MoMEMta: a modular proxy of the Matrix Element Method
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

#include <momemta/impl/ValueProxy_fwd.h>

/**
 * \brief A class representing a value produced by a module
 *
 * This class act as a proxy between the user and the value, providing a unique interface
 * to access values produced by a module, indexed or not.
 */
template <typename T>
class Value {
public:

    Value() = default;
    Value(const Value&) = default;
    Value(Value&&) = default;
    Value<T>& operator=(const Value<T>&) = default;

    const T& operator*() {
        return proxy->operator*();
    }

    const T& operator*() const {
        return proxy->operator*();
    }

    const T* operator->() {
        return proxy->operator->();
    }

    const T* operator->() const {
        return proxy->operator->();
    }

    const T* get() {
        return proxy->get();
    }

private:
    friend class Pool;

    // Only Pool can create a Value
    Value(std::shared_ptr<ValueProxy<const T>> impl) {
        proxy = impl;
    }

    std::shared_ptr<ValueProxy<const T>> proxy;
};