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
#include <momemta/impl/logger/formatter.h>
#include <momemta/impl/logger/sink.h>

#include <sstream>
#include <vector>

namespace logger {

class logger {
public:
    logger(sink_ptr single_sink);

    virtual ~logger();
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    bool should_log(logging::level::level_enum) const;
    void set_level(logging::level::level_enum);
    logging::level::level_enum level() const;

    void set_formatter(formatter_ptr);

    // automatically call flush() if message level >= log_level
    void flush_on(logging::level::level_enum log_level);

    virtual void flush();

    const std::vector<sink_ptr>& sinks() const;

    template <typename T> void log(logging::level::level_enum lvl, T&&);

protected:

    virtual void _sink_it(details::log_msg&);
    /// return true if the given message level should trigger a flush
    bool _should_flush_on(const details::log_msg&);

    std::vector<sink_ptr> _sinks;
    formatter_ptr _formatter;
    level_t _level;
    level_t _flush_level;
    std::atomic_size_t _index;
};

class ostream_wrapper {
public:
    ostream_wrapper(logger& l, logging::level::level_enum lvl);

    virtual ~ostream_wrapper();
    ostream_wrapper(const ostream_wrapper&) = delete;
    ostream_wrapper& operator=(const ostream_wrapper&) = delete;

    template <typename T> friend ostream_wrapper& operator<<(ostream_wrapper& w, T&& msg) {
        w._stream << msg;

        return w;
    }

    bool valid();
    void destroy();

private:
    bool _valid;
    ::logger::logger& _logger;
    ::logging::level::level_enum _lvl;
    std::ostringstream _stream;
};

logger_ptr get();

}

template <typename T>
void ::logger::logger::log(logging::level::level_enum lvl, T&& msg) {

    if (!should_log(lvl))
        return;

    ::logger::details::log_msg log_msg(lvl);
    log_msg.index = _index.fetch_add(1, std::memory_order_relaxed);
    log_msg.raw << msg;
    _sink_it(log_msg);
}