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
#include <string>
#include <vector>

class Module;


/**
 * \brief An execution path
 *
 * A Path represents an ordered sequence of modules, accessible using the Path::modules() method.
 *
 * \note A Path does not by itself execute any modules, this is left to the user of this class
 *
 * \sa Looper module
 */
class Path {
public:
    /**
     * \brief Create a new instance of Path
     *
     * \param path Description of
     */
    Path(const std::vector<std::shared_ptr<Module>>& modules);

    /**
     * \brief Create a new instance of Path from an existing instance
     *
     * \param path An existing instance of Path
     */
    Path(const Path&) = default;

    /**
     * \brief Create a new empty instance of Path
     */
    Path() = default;

    /**
     * \brief The sequence of modules of this execution Path
     *
     * The return value is only valid if resolved() returns `true`.
     *
     * \return Sequence of modules
     */
    const std::vector<std::shared_ptr<Module>>& modules() const;


private:
    std::vector<std::shared_ptr<Module>> modules_;
};