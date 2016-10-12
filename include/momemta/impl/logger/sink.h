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

#pragma once

#include <momemta/impl/logger/common.h>
#include <momemta/impl/logger/log_msg.h>

namespace logger {

namespace sinks {

class sink {
public:
    sink(): _level( logging::level::trace ) {}

    virtual ~sink() {}
    virtual void log(const details::log_msg& msg) = 0;
    virtual void flush() = 0;

    bool should_log(logging::level::level_enum msg_level) const;
    void set_level(logging::level::level_enum log_level);
    logging::level::level_enum level() const;

private:
    level_t _level;
};

}

}