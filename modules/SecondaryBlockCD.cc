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

#include <momemta/ParameterSet.h>
#include <momemta/Module.h>
#include <momemta/Solution.h>
#include <momemta/Math.h>
#include <momemta/InputTag.h>
#include <momemta/Types.h>

#include <Math/GenVector/VectorUtil.h>

/** \brief Secondary Block C/D, describing \f$s_{12} \to p_1 + p_2\f$
 *
 * This Secondary Block determine the energy \f$E_1\f$ of \f$p_1\f$ knowing the following quantities:
 * - \f$p_1\f$ angles and mass
 * - \f$p_2\f$
 * - \f$s_{12}\f$
 *
 * by solving the equation \f$s_{12} = (p_1 + p_2)^2\f$. It is useful if one wants to trade the degree of freedom \f$E_1\f$ for the propagator squared mass \f$s_{12}\f$, ensuring thus to probe phase space points close to the resonance. This is particularly interesting when the transfer function on \f$E_1\f$ is broad and would often lead to phase-space points where \f$(p_1 + p_2)\f$ mass are in the tail of the resonance Breit-Wigner. NB: this secondary block can be fed either with the output of the `BreitWignerGenerator` module if one wants to integrate over the mass resonance or with the output of the `NarrowWidthApproximation` module if one wants a fixed mass.
 *
 * The standard phase-space parametrization associated with this block reads: \f$ \frac{d^3p_1}{(2\pi)^32E_1}\f$. The procedure implies a change of variable leading to the following phase-space parametrization: \f$ \frac{1}{(2\pi)^32E_1}d\phi_1d\theta_1ds_{12}\times J \f$, where \f$ \theta_1 \f$ and \f$ \phi_1 \f$ denote the polar and azimutal angles of the decay product for which we want to determine the energy. The jacobian \f$ J \f$ of the transformation is given by \f$ J = \frac{E_1 |\boldsymbol{p_1}|^2 \sin\theta_1}{2|E_2|\boldsymbol{p_1}|-E_1|\boldsymbol{p_2}|\cos\theta_{12}|} \f$. There are up to two solutions for \f$E_1\f$.
 *
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Global parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `energy` | double | Collision energy. |
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `s12` | double | Squared invariant mass of the propagator (GeV\f$^2\f$). |
 *   | `p1` | LorentzVector | Reconstructed LorentzVector of the decay product for which we want to fix the energy. This LorentzVector will be used only to retrieve the particle angles and mass, its energy will be disregarded. |
 *   | `p2` | LorentzVector | Parton level LorentzVector of the decay product for which we already know everything. It will be used together with \f$s_{12}\f$ and the angles of \f$p_1^{reco}\f$ to fix \f$p_1^{gen}\f$ energy.|
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the above system for `p1`. Each solution embed the parton level LorentzVector of the reconstructed decay product and the jacobian associated with the change of variable mentioned above.|
 *
 * \note This block has been validated and is safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */
class SecondaryBlockCD: public Module {
    public:

        SecondaryBlockCD(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
                sqrt_s = parameters.globalParameters().get<double>("energy");

                s12 = get<double>(parameters.get<InputTag>("s12"));
                
                p1 = get<LorentzVector>(parameters.get<InputTag>("p1"));
                p2 = get<LorentzVector>(parameters.get<InputTag>("p2"));
            };

        virtual Status work() override {

            solutions->clear();

            // Don't spend time on unphysical part of phase-space
            if (*s12 > SQ(sqrt_s) || *s12 < p2->M2() || *s12 < p1->M2())
               return Status::NEXT;

            std::vector<double> E1_solutions; // up to two solutions
            const double theta1 = p1->Theta();
            const double phi1 = p1->Phi();
            const double m1 = p1->M();

            const double E2 = p2->E();
            const double norm2 = p2->P();
            const double m2 = p2->M();

            const double cos_theta12 = ROOT::Math::VectorUtil::CosTheta(*p1, *p2);

            // Equation to be solved for E1 : s12 = (p1+p2)^2 ==> [ 4*SQ(cos_theta12)*SQ(norm2)-4*SQ(E2) ] * SQ(E1) + [ 4*(s12 - SQ(m1) - SQ(m2))*E2 ] * E1 + 4*SQ(m1)*SQ(cos_theta12)*(SQ(m2)-SQ(E2) = 0)
            const double quadraticTerm = 4 * SQ(E2) - 4 * SQ(norm2) * SQ(cos_theta12);
            const double linearTerm = 4 * E2 * (SQ(m1) + SQ(m2) - *s12);
            const double indepTerm = SQ(SQ(m1) + SQ(m2) - *s12) + 4 * SQ(m1) * SQ(norm2) * SQ(cos_theta12);

            bool foundSolution = solveQuadratic(quadraticTerm, linearTerm, indepTerm, E1_solutions);

            if (!foundSolution) {
                return Status::NEXT;
            }

            for (const double& E1: E1_solutions) {
                // Skip unphysical solutions
                if (E1 <= 0 || E1 <= m1)
                    continue;
                
                // Avoid introducing spurious solution from the equation s12 = (p1+p2)^2 that has to be squared in the computation
                if ((SQ(m1) + SQ(m2) + 2 * E1 * E2 - *s12) * cos_theta12 < 0)
                    continue;
                
                LorentzVector gen_p1_sol;
                double norm1 = std::sqrt(SQ(E1) - SQ(m1));
                double gen_pt1_sol = norm1 * std::sin(theta1);
                gen_p1_sol.SetPxPyPzE(
                        gen_pt1_sol * std::cos(phi1),
                        gen_pt1_sol * std::sin(phi1),
                        norm1 * std::cos(theta1),
                        E1);
                
                // Compute jacobian
                double jacobian = std::abs( std::sin(theta1) * SQ(norm1) / (32 * CB(M_PI) * (norm1 * E2 - E1 * norm2 * cos_theta12)) );
                
                Solution s { {gen_p1_sol}, jacobian, true };
                solutions->push_back(s);
            }

            return (solutions->size() > 0) ? Status::OK : Status::NEXT;
        }

    private:
        double sqrt_s;

        // Inputs
        Value<double> s12;
        Value<LorentzVector> p1;
        Value<LorentzVector> p2;

        // Output
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(SecondaryBlockCD);
