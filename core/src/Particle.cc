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

#include <momemta/Particle.h>

namespace momemta {

Particle::Particle(const std::string& name_)
        : name(name_), type(0) {}

Particle::Particle(const std::string& name_, const LorentzVector& p4_)
        : name(name_), p4(p4_), type(0) {}

Particle::Particle(const std::string& name_, const LorentzVector& p4_, int64_t type_)
        : name(name_), p4(p4_), type(type_) {}

void swap(Particle& a, Particle& b) {
    using std::swap; // bring in swap for built-in types

    swap(a.name, b.name);
    swap(a.p4, b.p4);
    swap(a.type, b.type);
}

}