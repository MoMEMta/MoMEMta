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

#include <momemta/impl/logger/logger.h>

#include <momemta/impl/logger/common.h>

logger::logger::logger(::logger::sink_ptr single_sink) {
    _level = logging::level::info;
    _flush_level = logging::level::off;
    _sinks.push_back(single_sink);
    _formatter = std::make_shared<::logger::full_formatter>();
    _index.store(1, std::memory_order_relaxed);
}

logger::logger::~logger() = default;

void logger::logger::set_level(logging::level::level_enum log_level) {
    _level.store(log_level);
}
void logger::logger::flush_on(logging::level::level_enum log_level) {
    _flush_level.store(log_level);
}

void logger::logger::set_formatter(formatter_ptr formatter) {
    _formatter = formatter;
}

logging::level::level_enum logger::logger::level() const {
    return static_cast<logging::level::level_enum>(_level.load(std::memory_order_relaxed));
}

bool logger::logger::should_log(logging::level::level_enum msg_level) const {
    return msg_level >= _level.load(std::memory_order_relaxed);
}

// protected virtual called at end of each user log call (if enabled) by the line_logger
void logger::logger::_sink_it(::logger::details::log_msg& msg) {
    _formatter->format(msg);

    for (auto &sink : _sinks) {
        if (sink->should_log( msg.level)) {
            sink->log(msg);
        }
    }

    if (_should_flush_on(msg))
        flush();
}

void logger::logger::flush() {
    for (auto& sink : _sinks)
        sink->flush();
}


bool logger::logger::_should_flush_on(const ::logger::details::log_msg &msg) {
    const auto flush_level = _flush_level.load(std::memory_order_relaxed);
    return (msg.level >= flush_level) && (msg.level != logging::level::off);
}

const std::vector<logger::sink_ptr>& logger::logger::sinks() const {
    return _sinks;
}

::logger::ostream_wrapper::ostream_wrapper(::logger::logger& l, logging::level::level_enum lvl):
        _valid(true),
        _logger(l),
        _lvl(lvl) {
    // Empty
}

::logger::ostream_wrapper::~ostream_wrapper() {
    _logger.log(_lvl, _stream.str());
    _valid = false;
}

bool ::logger::ostream_wrapper::valid() {
    return _valid;
}

void ::logger::ostream_wrapper::destroy() {
    _valid = false;
}