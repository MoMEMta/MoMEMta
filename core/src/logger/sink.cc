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

// Heavily inspired by spdlog, Copyright(c) 2015 Gabi Melman

#include <momemta/impl/logger/sink.h>

namespace logger {

namespace sinks {

bool sink::should_log(logging::level::level_enum msg_level) const {
    return msg_level >= _level.load(std::memory_order_relaxed);
}

void sink::set_level(logging::level::level_enum log_level) {
    _level.store(log_level);
}

logging::level::level_enum sink::level() const {
    return static_cast<logging::level::level_enum>(_level.load(std::memory_order_relaxed));
}

}

}