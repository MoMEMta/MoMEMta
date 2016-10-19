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

#include <momemta/impl/logger/common.h>
#include <momemta/impl/logger/logger.h>

#define MERGE(a, b) a##b
#define UNIQUE_WRAPPER_NAME_INTERNAL(a) MERGE(_momemta_logger_, a)
#define UNIQUE_WRAPPER_NAME UNIQUE_WRAPPER_NAME_INTERNAL(__LINE__)

#define LOG_INTERNAL(lvl, wrapper) \
    for (::logger::ostream_wrapper wrapper(*::logger::get(), ::logging::level::lvl); wrapper.valid(); wrapper.destroy()) \
        wrapper

#define LOG(lvl) \
    LOG_INTERNAL(lvl, UNIQUE_WRAPPER_NAME)

namespace logging {
    void set_level(::logging::level::level_enum lvl);
}
