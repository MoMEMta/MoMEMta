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

template<typename T> const T& InputTag::get() const {
    if (! resolved) {
        throw tag_not_resolved_error("You must call 'resolve' once before calling 'get'");
    }

    if (content.empty()) {
        // Request the real content to the pool
        content = pool->raw_get(*this);
    }

    if (isIndexed()) {
        auto ptr = boost::any_cast<std::shared_ptr<std::vector<T>>>(content);
        return (*ptr)[index];
    } else {
        auto ptr = boost::any_cast<std::shared_ptr<T>>(content);
        return (*ptr);
    }
}
