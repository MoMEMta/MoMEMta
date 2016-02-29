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


#include <momemta/InputTag.h>
#include <momemta/Pool.h>

std::vector<std::string> split(const std::string& s, const std::string& delimiters) {

    std::vector<std::string> result;

    size_t current;
    size_t next = -1;
    do
    {
        next = s.find_first_not_of(delimiters, next + 1);
        if (next == std::string::npos)
            break;
        next -= 1;

        current = next + 1;
        next = s.find_first_of(delimiters, current);
        result.push_back(s.substr(current, next - current));
    }
    while (next != std::string::npos);

    return result;
}

void InputTag::resolve(PoolPtr pool) const {
    if (resolved)
        return;

    content = pool->raw_get(*this);
    resolved = true;
}
