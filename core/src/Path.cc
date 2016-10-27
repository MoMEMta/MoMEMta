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

#include <momemta/Path.h>

#include <momemta/Module.h>

Path::Path(PathElementsPtr elements) {
    elements_ = elements;
}

const std::vector<ModulePtr>& Path::modules() const {
    if (!frozen) {
        if (! elements_ || !elements_->resolved)
            throw std::runtime_error("You can access modules inside a path only if the elements are resolved. Maybe you forgot to call `freeze`?");

        return elements_->modules;
    }

    return modules_;
}

void Path::freeze() {

    if (frozen)
        return;

    frozen = true;
    modules_ = elements_->modules;
    elements_ = nullptr;
}