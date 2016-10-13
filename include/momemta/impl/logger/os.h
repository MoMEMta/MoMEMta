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

#include <chrono>
#include <ctime>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>

namespace logger {

namespace details {

namespace os {

inline log_clock::time_point now() {
    return log_clock::now();
}

inline std::tm localtime(const std::time_t& time_tt) {
    std::tm tm;
    localtime_r(&time_tt, &tm);

    return tm;
}

inline std::tm localtime() {
    std::time_t now_t = time(nullptr);
    return localtime(now_t);
}

inline size_t thread_id() {
    return static_cast<size_t>(syscall(SYS_gettid));
}

#if !defined (LOGGER_EOL)
#define LOGGER_EOL "\n"
#endif

constexpr static const char* eol = LOGGER_EOL;
constexpr static int eol_size = sizeof(LOGGER_EOL) - 1;

}

}

}