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

#include <algorithm>
#include <string>
#include <vector>

#include <momemta/Types.h>

/*!
 * \brief Convert a LorentzVector to a vector of real number.
 *
 * \param v The LorentzVector to convert
 *
 * \return a vector of real number containing 4 entries: [E, Px, Py, Pz]
 */
template <class T> std::vector<typename T::Scalar> toVector(const T& v) {
    return {v.E(), v.Px(), v.Py(), v.Pz()};
}

/*!
 * Compute the array of permutation needed to go from vector `from` to vector `to`
 *
 * \from The array before sorting
 * \to The array after sorting
 *
 * \return An array of the same size than `from` containing the permutation to apply to go
 * from `from` to `to`
 */
template <typename T>
std::vector<std::size_t> get_permutations(const std::vector<T>& from, const std::vector<T>& to) {

    std::vector<std::size_t> p(from.size());

    std::transform(from.begin(), from.end(), p.begin(), [&from, &to](const T& item) -> std::size_t {
        return std::distance(to.begin(), std::find(to.begin(), to.end(), item));
    });

    return p;
}

/*!
 * Apply a set of permutations to a vector
 *
 * \vec The vector to sort
 * \p The set of permutation to apply
 *
 * \return A new vector with all the permutations applied
 */
template <typename T>
void apply_permutations(std::vector<T>& vec, std::vector<std::size_t> const& p) {
    std::vector<T> sorted_vec(p.size());
    std::transform(p.begin(), p.end(), sorted_vec.begin(), [&vec](int i) { return vec[i]; });

    vec = sorted_vec;
}

/*!
 * Generate a random Lorentz vector
 *
 * \m The mass of the vector (default 0)
 * \maxE The maximum energy of the vector
 *
 * \return The random LorentzVector
 */
LorentzVector getRandom4Vector(double maxE, double m=0);

std::string demangle(const char* name);

namespace cuba {

inline unsigned int createFlagsBitset(char verbosity, bool subregion, bool retainStateFile,
                                      unsigned int level, bool smoothing,
                                      bool takeOnlyGridFromFile) {

    unsigned int flags = 0;

    static unsigned int opt_subregion = 0x04;            // bit 2 (=4)
    static unsigned int opt_smoothing = 0x08;            // bit 3 (=8)
    static unsigned int opt_retainStateFile = 0x10;      // bit 4 (=16)
    static unsigned int opt_takeOnlyGridFromFile = 0x20; // bit 5 (=32)

    level <<= 8;                // bits 8-31
    flags |= level | verbosity; // verbosity: bits 0-1
    if (subregion)
        flags |= opt_subregion;
    if (!smoothing)
        flags |= opt_smoothing; // careful true-false inverted
    if (retainStateFile)
        flags |= opt_retainStateFile;
    if (takeOnlyGridFromFile)
        flags |= opt_takeOnlyGridFromFile;

    return flags;
}
}
