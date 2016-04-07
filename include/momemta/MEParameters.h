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

#ifndef MOMEMTA_MEPARAMETERS_H
#define MOMEMTA_MEPARAMETERS_H

#include <unordered_map>

namespace momemta {

    class MEParameters {
        public:
            MEParameters() = default;
            virtual ~MEParameters() {};

            virtual void cacheParameters() = 0;
            virtual void cacheCouplings() = 0;

            virtual void updateParameters() = 0; 
            virtual void updateCouplings() = 0;

        protected:
            std::unordered_map<std::string, double> m_card_parameters;
    };

}

#endif
