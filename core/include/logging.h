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

#include <spdlog/spdlog.h>

namespace logging {

inline std::string color(uint16_t color) {
    return "\033[" + std::to_string(color) + "m";
}

class colored_formatter: public spdlog::formatter
{
    void format(spdlog::details::log_msg& msg) override
    {
#ifndef SPDLOG_NO_DATETIME
        auto tm_time = spdlog::details::os::localtime(spdlog::log_clock::to_time_t(msg.time));
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

        msg.formatted << '[' << static_cast<unsigned int>(tm_time.tm_year + 1900) << '-'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mon + 1), 2, '0') << '-'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mday), 2, '0') << ' '
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_hour), 2, '0') << ':'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_min), 2, '0') << ':'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_sec), 2, '0') << '.'
                      << fmt::pad(static_cast<unsigned int>(millis), 3, '0') << "] ";

//no datetime needed
#else
        (void)tm_time;
#endif

#ifndef SPDLOG_NO_NAME
        msg.formatted << '[' << msg.logger_name << "] ";
#endif

        msg.formatted << '[';
        switch (msg.level) {
            case spdlog::level::trace:
            case spdlog::level::debug:
                msg.formatted << color(90);
                break;
            case spdlog::level::warn:
                msg.formatted << color(33);
                break;
            case spdlog::level::err:
            case spdlog::level::critical:
            case spdlog::level::alert:
            case spdlog::level::emerg:
                msg.formatted << color(31);
                break;
            default:
                break;
        }
        msg.formatted << spdlog::level::to_str(msg.level) << color(0) << "] ";
        msg.formatted << fmt::StringRef(msg.raw.data(), msg.raw.size());
        msg.formatted << spdlog::details::os::eol();
    }
};

template <class Mutex> class colored_stdout_sink {
  public:

    static std::shared_ptr<spdlog::sinks::stdout_sink<Mutex>> instance() {
        static std::shared_ptr<spdlog::sinks::stdout_sink<Mutex>> instance = std::make_shared<spdlog::sinks::stdout_sink<Mutex>>();
        instance->set_formatter(std::make_shared<colored_formatter>());
        return instance;
    }
};

typedef colored_stdout_sink<spdlog::details::null_mutex> colored_stdout_sink_st;

std::shared_ptr<spdlog::logger>& get();

}

#define LOG(severity) ::logging::get()->severity()

#define LOGGER ::logging::get()

#define TRACE(...) SPDLOG_TRACE(::logging::get(), __VA_ARGS__)
