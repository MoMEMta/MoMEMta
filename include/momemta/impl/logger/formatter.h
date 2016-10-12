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

#include <momemta/impl/logger/log_msg.h>

#include <chrono>
#include <iomanip>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace logger {

class formatter {
public:
    virtual ~formatter() {}
    virtual void format(details::log_msg& msg) = 0;
};

class full_formatter: public formatter {
public:
    void format(details::log_msg& msg) override;

protected:
    virtual void format_level(details::log_msg& msg);
};

class ansi_color_full_formatter: public full_formatter {
public:
    /// Formatting codes
    const std::string reset      = "\033[00m";
    const std::string bold       = "\033[1m";
    const std::string dark       = "\033[2m";
    const std::string underline  = "\033[4m";
    const std::string blink      = "\033[5m";
    const std::string reverse    = "\033[7m";
    const std::string concealed  = "\033[8m";

    // Foreground colors
    const std::string grey       = "\033[90m";
    const std::string red        = "\033[31m";
    const std::string green      = "\033[32m";
    const std::string yellow     = "\033[33m";
    const std::string blue       = "\033[34m";
    const std::string magenta    = "\033[35m";
    const std::string cyan       = "\033[36m";
    const std::string white      = "\033[37m";

    /// Background colors
    const std::string on_grey    = "\033[40m";
    const std::string on_red     = "\033[41m";
    const std::string on_green   = "\033[42m";
    const std::string on_yellow  = "\033[43m";
    const std::string on_blue    = "\033[44m";
    const std::string on_magenta = "\033[45m";
    const std::string on_cyan    = "\033[46m";
    const std::string on_white = "\033[47m";

protected:
    void format_level(details::log_msg& msg) override;
};

}