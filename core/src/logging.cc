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

#include <momemta/Logging.h>

#include <memory>
#include <unistd.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;

namespace logging {

typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;

namespace {

inline std::string color(uint16_t color) { return "\033[" + std::to_string(color) + "m"; }

void colored_formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm) {
    static auto date_time_formatter = expr::stream << expr::format_date_time<boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");

    strm << boost::log::extract<unsigned int>("LineID", rec)
        << ": [";
    date_time_formatter(rec, strm);
    strm << "] [";

    auto severity = rec[boost::log::trivial::severity];

    switch (*severity) {
        case boost::log::trivial::trace:
        case boost::log::trivial::debug:
            strm << color(90);
            break;
        case boost::log::trivial::info:
            strm << color(34);
            break;
        case boost::log::trivial::warning:
            strm << color(33);
            break;
        case boost::log::trivial::error:
        case boost::log::trivial::fatal:
            strm << color(31);
            break;
        default:
            break;
    }

    strm << severity
         << color(0)
         << "]  "
         << rec[expr::message];
}

static std::shared_ptr<logger> init_logger() {
    bool in_terminal = isatty(fileno(stderr));

    // Create sink
    boost::shared_ptr<text_sink> sink = boost::log::add_console_log();
    sink->locked_backend()->auto_flush(true);
    if (in_terminal) {
        sink->set_formatter(&colored_formatter);
    } else {

        // Log format
        auto formatter =
            expr::stream
            << expr::attr<unsigned int>("LineID")
            << ": ["
            << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            /* << "] ["
               << expr::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID") */
            << "] ["
            << boost::log::trivial::severity
            << "]  "
            << expr::message;

        sink->set_formatter(formatter);
    }

    boost::log::add_common_attributes();

    return std::make_shared<logger>();
}
}

std::shared_ptr<logger>& get() {
    static std::shared_ptr<logger> s_logger = init_logger();
    return s_logger;
}

void set_level(boost::log::trivial::severity_level lvl) {
    ::boost::log::core::get()->set_filter(
            boost::log::trivial::severity >= lvl
            );
}

}
