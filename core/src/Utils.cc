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

#include <random>

#include <momemta/Utils.h>
#include <momemta/Math.h>

LorentzVector getRandom4Vector(double maxE, double m/*=0*/) {
    static std::mt19937_64 generator;
    std::uniform_real_distribution<double> dist(-maxE, maxE);

    while (true) {
        const double px = dist(generator);
        const double py = dist(generator);
        const double pz = dist(generator);
        const double E = std::sqrt(SQ(m) + SQ(px) + SQ(py) + SQ(pz));
        LorentzVector p4(px, py, pz, E);
        if (E < maxE && p4.M() >= 0) {
            return p4;
        }
    }
}

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>

std::string demangle(const char* name) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(name, NULL, NULL, &status),
                                               std::free};

    return (status == 0) ? res.get() : name;
}

#else

// does nothing if not g++
std::string demangle(const char* name) { return name; }

#endif
