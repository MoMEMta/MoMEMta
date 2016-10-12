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

#include <momemta/impl/logger/formatter.h>

#include <momemta/impl/logger/os.h>

namespace logger {

void full_formatter::format(details::log_msg& msg) {
    auto tm_time = details::os::localtime(log_clock::to_time_t(msg.time));

    auto duration = msg.time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    msg.formatted << msg.index << ": [" << static_cast<unsigned int>(tm_time.tm_year + 1900) << '-'
                  << std::setfill('0')
                  << std::setw(2) << static_cast<unsigned int>(tm_time.tm_mon + 1) << '-'
                  << std::setw(2) << static_cast<unsigned int>(tm_time.tm_mday) << ' '
                  << std::setw(2) << static_cast<unsigned int>(tm_time.tm_hour) << ':'
                  << std::setw(2) << static_cast<unsigned int>(tm_time.tm_min) << ':'
                  << std::setw(2) << static_cast<unsigned int>(tm_time.tm_sec) << '.'
                  << std::setw(3) << static_cast<unsigned int>(millis) << "] ";

    msg.formatted << '[';
    format_level(msg);
    msg.formatted << "] ";

    msg.formatted << msg.raw.str();

    msg.formatted.write(details::os::eol, details::os::eol_size);
}

void full_formatter::format_level(details::log_msg& msg) {
    msg.formatted << ::logging::level::to_str(msg.level);
}

void ansi_color_full_formatter::format_level(details::log_msg& msg) {
    switch (msg.level) {
    case logging::level::trace:
    case logging::level::debug:
        msg.formatted << grey;
        break;
    case logging::level::info:
        msg.formatted << blue;
        break;
    case logging::level::warning:
        msg.formatted << yellow << bold;
        break;
    case logging::level::error:
        msg.formatted << red << bold;
        break;
    case logging::level::fatal:
        msg.formatted << bold << on_red;
        break;
    case logging::level::off:
        msg.formatted << reset;
    default:
        break;
    }

    msg.formatted << ::logging::level::to_str(msg.level) << reset;
}

}