#ifndef _INC_UTILS
#define _INC_UTILS

#include <cmath>
#include <vector>
#include <algorithm>

#include <Types.h>

#define SQ(x) (x*x)
#define CB(x) (x*x*x)
#define QU(x) (x*x*x*x)

//#define M_PI 3.1415927410125732421875

template<typename T> T sign(const T x){
    if(x > 0)
        return 1;
    else if(!x)
        return 0;
    else
        return -1;
}

// Used to compute Jacobian for Transfer Function
inline double dE_over_dP(const LorentzVector& v){
    const double rad = SQ(v.E()) - SQ(v.M());
    if (rad <= 0)
        return 0.;
    else
        return v.E() / std::sqrt(rad);
}

// If the NWA is used for a particle, a multiplication factor has to be introduced
// because of the integrated-out delta function
inline double jacobianNWA(const double mass, const double width){
    return ( M_PI/2. + atan(mass / width) ) * mass * width;
}

// Compute cos(x +- 2*pi/3) in a more "analytical" way (pm = +- 1)
// Useful for solveCubic
inline double cosXpm2PI3(const double x, const double pm){
    return -0.5*( cos(x) + pm * sin(x) * sqrt(3.) );
}

// Finds the real solutions to a*x^2 + b*x + c = 0
// Uses a numerically more stable way than the "classroom" method.
// Handles special cases a=0 and/or b=0.
// Appends the solutions to the std::vector roots, making no attempt to check whether the vector is empty.
// Double roots are present twice.
// If verbose is true (default is false), the solutions are printed,
// as well as the polynomial evaluated on these solutions
//
// See https://fr.wikipedia.org/wiki/Équation_du_second_degré#Calcul_numérique
bool solveQuadratic(const double a, const double b, const double c,
        std::vector<double>& roots,
        bool verbose = false
        );

// Finds the real solutions to a*x^3 + b*x^2 + c*x + d = 0
// Handles special case a=0.
// Appends the solutions to the std::vector roots, making no attempt to check whether the vector is empty.
// Multiple roots appear multiple times.
// If verbose is true (default is false), the solutions are printed,
// as well as the polynomial evaluated on these solutions
//
// Inspired by "Numerical Recipes" (Press, Teukolsky, Vetterling, Flannery), 2007 Cambridge University Press
bool solveCubic(const double a, const double b, const double c, const double d,
        std::vector<double>& roots,
        bool verbose = false
        );

// Finds the real solutions to a*x^4 + b*x^3 + c*x^2 + d*x + e = 0
// Handles special case a=0.
// Appends the solutions to the std::vector roots, making no attempt to check whether the vector is empty.
// Multiple roots appear multiple times.
// If verbose is true (default is false), the solutions are printed,
// as well as the polynomial evaluated on these solutions
//
// See https://en.wikipedia.org/wiki/Quartic_function#Solving_by_factoring_into_quadratics
//    https://fr.wikipedia.org/wiki/Méthode_de_Descartes
//
// The idea is to make a change of variable to eliminate the term in x^3, which gives a "depressed quartic",
// then to try and factorize this quartic into two quadratic equations, which each then gives up to two roots.
// The factorization relies on solving a cubic equation (which is always possible),
// then taking the square root of one of these solution (ie there must be a positive solution).
bool solveQuartic(const double a, const double b, const double c, const double d, const double e,
        std::vector<double>& roots,
        bool verbose = false
        );

// Solves the system:
// a20*E1^2 + a02*E2^2 + a11*E1*E2 + a10*E1 + a01*E2 + a00 = 0
// b20*E1^2 + b02*E2^2 + b11*E1*E2 + b10*E1 + b01*E2 + b00 = 0
// Which corresponds to finding the intersection points of two conics.
// Appends the (x,y) solutions to the std::vectors E1, E2, making no attempt to check
// whether these vectors are empty.
// In most cases it simply comes down to solving a quartic equation:
//   - eliminate the E1^2 term
//   - solve for E1 (linear!)
//   - inserting back gives a quartic function of E2
//   - find solutions for E1
// The procedure becomes tricky in some special cases
// (intersections aligned along x- or y-axis, degenerate conics, ...)
bool solve2Quads(const double a20, const double a02, const double a11, const double a10, const double a01, const double a00,
        const double b20, const double b02, const double b11, const double b10, const double b01, const double b00,
        std::vector<double>& E1, std::vector<double>& E2,
        bool verbose = false
        );

// Solves the system:
// a11*E1*E2 + a10*E1 + a01*E2 + a00 = 0
// b11*E1*E2 + b10*E1 + b01*E2 + b00 = 0
// Which corresponds to finding the intersection points of two conics.
// Appends the (x,y) solutions to the std::vectors E1, E2, making no attempt to check
// whether these vectors are empty.
bool solve2QuadsDeg(const double a11, const double a10, const double a01, const double a00,
        const double b11, const double b10, const double b01, const double b00,
        std::vector<double>& E1, std::vector<double>& E2,
        bool verbose = false
        );

// Solves the system:
// a10*E1 + a01*E2 + a00 = 0
// b10*E1 + b01*E2 + b00 = 0
// Appends the (x,y) solutions to the std::vectors E1, E2, making no attempt to check
// whether these vectors are empty.
bool solve2Linear(const double a10, const double a01, const double a00,
        const double b10, const double b01, const double b00,
        std::vector<double>& E1, std::vector<double>& E2,
        bool verbose = false);


double BreitWigner(const double s, const double m, const double g);

/*!
 * Convert a LorentzVector to a vector of real number.
 *
 * @v   The LorentzVector to convert
 *
 * @returns a vector of real number containing 4 entries: [E, Px, Py, Pz]
 */
template<class T>
std::vector<typename T::Scalar> toVector(const T& v) {
    return {v.E(), v.Px(), v.Py(), v.Pz()};
}

/*!
 * Compute the array of permutation needed to go from vector `from` to vector `to`
 *
 * @from The array before sorting
 * @to The array after sorting
 *
 * @return An array of the same size than `from` containing the permutation to apply to go
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
 * @vec The vector to sort
 * @p The set of permutation to apply
 *
 * @return A new vector with all the permutations applied
 */
template <typename T>
void apply_permutations(std::vector<T>& vec, std::vector<std::size_t> const& p) {
    std::vector<T> sorted_vec(p.size());
    std::transform(p.begin(), p.end(), sorted_vec.begin(), [&vec](int i) {
                return vec[i];
            });

    vec = sorted_vec;
}

#endif
