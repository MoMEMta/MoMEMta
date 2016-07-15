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

#include <cmath>
#include <vector>

/// Compute \f$ x^2 \f$
#define SQ(x) ((x) * (x))
/// Compute \f$ x^3 \f$
#define CB(x) ((x) * (x) * (x))
/// Compute \f$ x^4 \f$
#define QU(x) ((x) * (x) * (x) * (x))

/**
 * \file
 * \brief Mathematical functions
 */

/**
 * \brief Sign function
 *
 * \return 1 if \p x is positive, -1 if \p x is negative, or 0 if x is null
 */
template <typename T> T sign(const T& x) {
    if (x > 0)
        return 1;
    else if (!x)
        return 0;
    else
        return -1;
}

/**
 * \brief Compute \f$ \frac{dE}{dP} \f$
 */
template <typename T> inline double dP_over_dE(const T& v) {
    const double rad = SQ(v.E()) - SQ(v.M());
    if (rad <= 0)
        return 0.;
    else
        return v.E() / std::sqrt(rad);
}

// If the NWA is used for a particle, a multiplication factor has to be introduced
// because of the integrated-out delta function
inline double jacobianNWA(const double mass, const double width) {
    return (M_PI / 2. + std::atan(mass / width)) * mass * width;
}

// Compute cos(x +- 2*pi/3) in a more "analytical" way (pm = +- 1)
// Useful for solveCubic
inline double cosXpm2PI3(const double x, const double pm) {
    return -0.5 * (std::cos(x) + pm * std::sin(x) * std::sqrt(3.));
}

/**
 * \brief Finds the real solutions to \f$ a*x^2 + b*x + c = 0 \f$
 *
 * Uses a numerically more stable way than the "classroom" method.
 * Handles special cases \f$ a = 0 \f$ and / or \f$ b = 0 \f$.
 * Appends the solutions to \p roots, making no attempt to check whether the vector is empty.
 * \note Double roots are present twice.
 * See https://fr.wikipedia.org/wiki/Équation_du_second_degré#Calcul_numérique
 *
 * \param a, b, c Coefficient of quadratic equation
 * \param[out] roots Roots of equation
 * \param verbose If true, print the solution of the equation
 *
 * \return True if a solution has been found, false otherwise
 */
bool solveQuadratic(const double a, const double b, const double c, std::vector<double>& roots,
                    bool verbose = false);

/**
 * \brief Finds the real solutions to \f$ a*x^3 + b*x^2 + c*x + d = 0 \f$
 *
 * Handles special cases \f$ a = 0 \f$.
 *
 * Appends the solutions to \p roots, making no attempt to check whether the vector is empty.
 *
 * \note Multiple roots are present multiple times.
 *
 * \note Inspired by "Numerical Recipes" (Press, Teukolsky, Vetterling, Flannery), 2007 Cambridge University Press
 *
 * \param a, b, c, d Coefficient of the equation
 * \param[out] roots Roots of equation
 * \param verbose If true, print the solution of the equation
 *
 * \return True if a solution has been found, false otherwise
 */
bool solveCubic(const double a, const double b, const double c, const double d,
                std::vector<double>& roots, bool verbose = false);

// Finds the real solutions to a*x^4 + b*x^3 + c*x^2 + d*x + e = 0
// Handles special case a=0.
// Appends the solutions to the std::vector roots, making no attempt to check whether the vector is
// empty.
// Multiple roots appear multiple times.
// If verbose is true (default is false), the solutions are printed,
// as well as the polynomial evaluated on these solutions
//
// See https://en.wikipedia.org/wiki/Quartic_function#Solving_by_factoring_into_quadratics
//    https://fr.wikipedia.org/wiki/Méthode_de_Descartes
//
// .

/**
 * \brief Finds the real solutions to \f$ a*x^4 + b*x^3 + c*x^2 + d*x + e = 0 \f$
 *
 * Handles special cases \f$ a = 0 \f$.
 *
 * Appends the solutions to \p roots, making no attempt to check whether the vector is empty.
 *
 * The idea is to make a change of variable to eliminate the term in \f$ x^3 \f$, which gives a *depressed quartic*,
 * then to try and factorize this quartic into two quadratic equations, which each then gives up to two roots.
 * The factorization relies on solving a cubic equation (which is always possible), then taking the square root
 * of one of these solution (ie there must be a positive solution)
 *
 * \note Multiple roots are present multiple times.
 *
 * \note See https://en.wikipedia.org/wiki/Quartic_function#Solving_by_factoring_into_quadratics and 
 *       https://fr.wikipedia.org/wiki/Méthode_de_Descartes
 *
 * \param a, b, c, d, e Coefficient of the equation
 * \param[out] roots Roots of equation
 * \param verbose If true, print the solution of the equation
 *
 * \return True if a solution has been found, false otherwise
 */
bool solveQuartic(const double a, const double b, const double c, const double d, const double e,
                  std::vector<double>& roots, bool verbose = false);

/**
 * \brief Solve a system of two quadratic equations
 *
 * Solves the system:
 * \f[
 * \begin{align*}
 *   a_{20}E_1^2 + a_{02}E_2^2 + a_{11}E_1E_2 + a_{10}E_1 + a_{01}E_2 + a_{00} &= 0 \\
 *   b_{20}E_1^2 + b_{02}E_2^2 + b_{11}E_1E_2 + b_{10}E_1 + b_{01}E_2 + b_{00} &= 0
 *   \end{align*}
 * \f]
 * 
 * Which corresponds to finding the intersection points of two conics.
 * 
 * Appends the (x,y) solutions to the std::vectors E1, E2, making no attempt to check
 * whether these vectors are empty.
 *
 * In most cases it simply comes down to solving a quartic equation:
 *   - eliminate the \f$ E_1^2 \f$ term
 *   - solve for \f$ E_1 \f$ (linear!)
 *   - inserting back gives a quartic function of \f$ E_2 \f$
 *   - find solutions for \f$ E_1 \f$
 *
 * The procedure becomes tricky in some special cases (intersections aligned along x- or y-axis,
 * degenerate conics, ...)
 */
bool solve2Quads(const double a20, const double a02, const double a11, const double a10,
                 const double a01, const double a00, const double b20, const double b02,
                 const double b11, const double b10, const double b01, const double b00,
                 std::vector<double>& E1, std::vector<double>& E2, bool verbose = false);

/**
 * \brief Solve a system of two degenerated quadratic equations
 *
 * Solves the system:
 * \f[
 * \begin{align*}
 *   a_{11}E_1E_2 + a_{10}E_1 + a_{01}E_2 + a_{00} &= 0 \\
 *   b_{11}E_1E_2 + b_{10}E_1 + b_{01}E_2 + b_{00} &= 0
 *   \end{align*}
 * \f]
 * 
 * Which corresponds to finding the intersection points of two conics.
 * 
 * Appends the (x,y) solutions to the std::vectors E1, E2, making no attempt to check
 * whether these vectors are empty.
 *
 */
bool solve2QuadsDeg(const double a11, const double a10, const double a01, const double a00,
                    const double b11, const double b10, const double b01, const double b00,
                    std::vector<double>& E1, std::vector<double>& E2, bool verbose = false);

/**
 * \brief Solve a system of two linear equations
 *
 * Solves the system:
 * \f[
 * \begin{align*}
 *   a_{10}E_1 + a_{01}E_2 + a_{00} &= 0 \\
 *   b_{10}E_1 + b_{01}E_2 + b_{00} &= 0
 *   \end{align*}
 * \f]
 * 
 * Appends the (x,y) solutions to the std::vectors E1, E2, making no attempt to check
 * whether these vectors are empty.
 */
bool solve2Linear(const double a10, const double a01, const double a00, const double b10,
                  const double b01, const double b00, std::vector<double>& E1,
                  std::vector<double>& E2, bool verbose = false);

/**
 * \brief A relativist Breit-Wigner distribution
 */
double BreitWigner(const double s, const double m, const double g);
