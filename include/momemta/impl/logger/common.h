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

#include <atomic>
#include <chrono>
#include <memory>

namespace logger {

class formatter;
class logger;

namespace sinks {
struct sink;
}

using formatter_ptr = std::shared_ptr<::logger::formatter>;
using level_t = std::atomic<int>;
using logger_ptr = std::shared_ptr<::logger::logger>;
using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr<sinks::sink>;

}

namespace logging {

namespace level {

typedef enum {
    trace = 0,
    debug = 1,
    info = 2,
    warning = 3,
    error = 4,
    fatal = 5,
    off = 6
} level_enum;

static const char* level_names[] { "trace", "debug", "info", "warning", "error", "fatal", "off" };

inline const char* to_str(logging::level::level_enum l) {
    return level_names[l];
}

}

}