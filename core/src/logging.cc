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


#include <logging.h>
#include <unistd.h>

namespace logging {
    namespace {
        static std::shared_ptr<spdlog::logger> instance() {
            bool in_terminal = isatty(fileno(stdout));
            std::shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_st("MoMEMta");

            if (in_terminal)
                logger->set_formatter(std::make_shared<colored_formatter>());

            return logger;
        }
    }

    std::shared_ptr<spdlog::logger>& get() {
        static std::shared_ptr<spdlog::logger> s_logger = instance();
        return s_logger;
    }
}
