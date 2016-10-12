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

#include <sstream>
#include <string>
#include <utility>

#include <momemta/impl/logger/common.h>
#include <momemta/impl/logger/os.h>

namespace logger {

namespace details {

struct log_msg {
    log_msg() = default;
    log_msg(logging::level::level_enum lvl) : level(lvl) {
        time = os::now();
        thread_id = os::thread_id();
    }

    log_msg(const log_msg& other) = delete;
    log_msg& operator=(log_msg&& other) = delete;
    log_msg(log_msg&& other) = delete;

    logging::level::level_enum level;
    size_t index;
    log_clock::time_point time;
    size_t thread_id;
    std::ostringstream raw;
    std::ostringstream formatted;
};

}

}
