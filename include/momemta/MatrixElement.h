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

#ifndef MOMEMTA_MATRIXELEMENT_H
#define MOMEMTA_MATRIXELEMENT_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <momemta/MEParameters.h>

class ConfigurationSet;

// FIXME Namespace or not?
namespace momemta {
    
    class MatrixElement {
        public:
            using Result = std::map<std::pair<int, int>, double>;

            MatrixElement() = default;
            virtual ~MatrixElement() {};
    
            virtual Result compute(
                    const std::vector<std::vector<double>>& initialMomenta,
                    const std::vector<std::pair<int, std::vector<double>>>& finalState
                    ) = 0;

            virtual std::shared_ptr<MEParameters> getParameters() = 0;
    };

}

#endif