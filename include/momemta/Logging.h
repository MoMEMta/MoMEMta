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

#define BOOST_LOG_DYN_LINK 1

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>

namespace logging {
typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger;

std::shared_ptr<logger>& get();

void set_level(boost::log::trivial::severity_level lvl);
}

#define LOG(lvl)\
    BOOST_LOG_STREAM_WITH_PARAMS(*::logging::get(),\
        (::boost::log::keywords::severity = ::boost::log::trivial::lvl))
