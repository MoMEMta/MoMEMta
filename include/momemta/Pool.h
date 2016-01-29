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
#include <unordered_map>

#include <boost/any.hpp>

#include <momemta/InputTag.h>

// A simple memory pool
class Pool {
    public:
        template<typename T> std::shared_ptr<const T> get(const InputTag& tag) {
            if (tag.isIndexed()) {
                throw std::invalid_argument("Indexed input tag cannot be passed as argument of the pool. Use the `get` function of the input tag to retrieve its content.");
            }

            auto it = m_pool.find(tag);
            if (it == m_pool.end())
                throw tag_not_found_error("No such tag in pool: " + tag.toString());

            boost::any& v = it->second;
            std::shared_ptr<T>& ptr = boost::any_cast<std::shared_ptr<T>&>(v);

            return std::const_pointer_cast<const T>(ptr);
        }

        template<typename T> std::shared_ptr<T> put(const InputTag& tag) {
            auto it = m_pool.find(tag);
            if (it != m_pool.end())
                throw duplicated_tag_error("A module already produced the tag '" + tag.toString() + "'");

            std::shared_ptr<T> ptr(new T());
            m_pool.emplace(tag, boost::any(ptr));

            return ptr;
        }

        void alias(const InputTag& from, const InputTag& to);

    private:
        friend class MoMEMta;

        class tag_not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class duplicated_tag_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        friend struct InputTag;

        boost::any raw_get(const InputTag&);

        Pool() = default;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        std::unordered_map<InputTag, boost::any> m_pool;
};

using PoolPtr = std::shared_ptr<Pool>;
