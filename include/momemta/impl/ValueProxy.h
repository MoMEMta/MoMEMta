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

#include <momemta/impl/ValueProxy_fwd.h>

#include <momemta/InputTag.h>
#include <momemta/Pool.h>

template <typename T> template <typename U> std::shared_ptr<const U> ValueProxy<T>::get_from_pool(const Pool* pool, const InputTag& tag) {
    return pool->raw_get<U>(tag);
}

template <typename T>
NonIndexedValueProxy<T>::NonIndexedValueProxy(const Pool* pool, const InputTag& tag) {
    raw_value = this->template get_from_pool<container_t>(pool, tag);
}

template <typename T>
T& NonIndexedValueProxy<T>::operator*() {
    return *raw_value;
}

template <typename T>
T *NonIndexedValueProxy<T>::operator->() {
    return get();
}

template <typename T>
T *NonIndexedValueProxy<T>::get() {
    return raw_value.get();
}

template <typename T>
IndexedValueProxy<T>::IndexedValueProxy(const Pool* pool, const InputTag& tag) {
    raw_value = this->template get_from_pool<container_t>(pool, tag);
    index = tag.index;
}

template <typename T>
T& IndexedValueProxy<T>::operator*() {
    return raw_value->operator[](index);
}

template <typename T>
T *IndexedValueProxy<T>::operator->() {
    return get();
}

template <typename T>
T *IndexedValueProxy<T>::get() {
    return &raw_value->operator[](index);
}

template <typename T>
std::shared_ptr<ValueProxy<T>> ValueProxy<T>::create(const Pool* pool, const InputTag& tag) {
    if (tag.isIndexed()) {
        return std::make_shared<IndexedValueProxy<T>>(pool, tag);
    } else
        return std::make_shared<NonIndexedValueProxy<T>>(pool, tag);
}