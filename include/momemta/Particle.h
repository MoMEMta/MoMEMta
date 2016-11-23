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

#include <momemta/Types.h>

#include <string>

namespace momemta {

/**
 * \brief Describe a reco particle. Used as input of MoMEMta::computeWeights
 */
struct Particle {
public:
    const std::string name;
    LorentzVector p4;
    int64_t type;

    Particle(const std::string& name_);
    Particle(const std::string& name_, const LorentzVector& p4);
    Particle(const std::string& name_, const LorentzVector& p4, int64_t type);

    Particle(const Particle&) = default;
    Particle(Particle&&) = default;
};

}