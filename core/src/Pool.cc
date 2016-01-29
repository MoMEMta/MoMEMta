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

boost::any Pool::raw_get(const InputTag& tag) {
    auto it = m_pool.find(tag);
    if (it == m_pool.end())
        throw tag_not_found_error("No such tag in pool: " + tag.toString());
    return it->second;
}

void Pool::alias(const InputTag& from, const InputTag& to) {
    if (from.isIndexed() || to.isIndexed()) {
        throw std::invalid_argument("Indexed input tag cannot be passed as argument of the pool. Use the `get` function of the input tag to retrieve its content.");
    }

    auto from_it = m_pool.find(from);
    if (from_it == m_pool.end())
        throw tag_not_found_error("No such tag in pool: " + from.toString());

    auto to_it = m_pool.find(to);
    if (to_it != m_pool.end())
        throw duplicated_tag_error("A module already produced the tag '" + to.toString() + "'");

    m_pool[to] = m_pool[from];
}
