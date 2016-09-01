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
#include <type_traits>
#include <vector>

class Pool;
struct InputTag;

template <typename T>
class ValueProxy {
public:
    virtual T& operator*() = 0;
    virtual T* operator->() = 0;
    virtual T* get() = 0;

    static std::shared_ptr<ValueProxy<T>> create(const Pool*, const InputTag&);

protected:
    template <typename U>
    std::shared_ptr<const U> get_from_pool(const Pool*, const InputTag&);
};

template <typename T>
class NonIndexedValueProxy: public ValueProxy<T> {
public:
    using container_t = typename std::remove_const<T>::type;

    NonIndexedValueProxy(const Pool*, const InputTag& tag);
    virtual T& operator*() override;
    virtual T* operator->() override;
    virtual T* get() override;

private:
    std::shared_ptr<const container_t> raw_value;
};

template <typename T>
class IndexedValueProxy: public ValueProxy<T> {
public:
    using container_t = std::vector<typename std::remove_const<T>::type>;

    IndexedValueProxy(const Pool*, const InputTag& tag);
    virtual T& operator*() override;
    virtual T* operator->() override;
    virtual T* get() override;

private:
    size_t index;
    std::shared_ptr<const container_t> raw_value;
};