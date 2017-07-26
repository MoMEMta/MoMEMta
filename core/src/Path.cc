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

#include <ExecutionPath.h>
#include <Path.h>

#include <momemta/Module.h>

#include <random>

ExecutionPath::ExecutionPath() {
    std::random_device rd;
    std::mt19937 random_engine(rd());
    id = boost::uuids::basic_random_generator<std::mt19937>(random_engine)();
}

Path::Path(const std::vector<std::shared_ptr<Module>>& modules) {
    modules_ = modules;
}

const std::vector<ModulePtr>& Path::modules() const {
    return modules_;
}
