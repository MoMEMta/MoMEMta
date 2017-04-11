/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2017  Universite catholique de Louvain (UCL), Belgium
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

/** \brief \f$\require{cancel}\f$ Secondary Block E, describing \f$s_{123} \to s_{12}(\to p_1 + p_2) + p_3 \f$
 *
 * This Secondary Block determines \f$p_1\f$'s and \f$p_2\f$'s energy using
 * - \f$p_1\f$ direction and mass
 * - \f$p_2\f$ direction, assuming \f$m_2=0\f$
 * - \f$p_3\f$
 * - \f$s_{12}\f$
 * - \f$s_{123}\f$
 *
 * by solving these equations, for which there are up to four solutions:
 *  - \f$s_{12} = (p_1 + p_2)^2\f$
 *  - \f$s_{123} = (p_1 + p_2 + p_3)^2\f$
 *  - \f$p_1^2 = m_1^2\f$
 *  - \f$p_2^2 = 0\f$.
 *
 * The standard phase-space parametrization associated with this block reads: \f$ \frac{d^3 p_1}{(2\pi)^3 2E_1} \frac{d^3 p_2}{(2\pi)^3 2E_2}\f$. The procedure implies a change of variable leading to the following phase-space parametrization: \f$ \frac{1}{(2\pi)^6 4 E_1 E_2} d\theta_1 d\phi_1 d\theta_3 d\phi_2 ds_{12} ds_{123} \times J \f$ (where \f$J\f$ is the jacobian associated with the transformation).
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
 *   | `s123` | double | Squared invariant mass of the propagator (GeV\f$^2\f$). |
 *   | `p1` <br/> `p2` | LorentzVector | LorentzVectors of the particles for which we want to compute the energy and momentum. They will be used only to retrieve the particles' direction (and mass for \f$p_1\f$).|
 *   | `p3` | LorentzVector | LorentzVector of the decay product used to fully reconstruct \f$p_1\f$ and \f$p_2\f$ (see above description).|
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the above equations. Each solution embed the parton level LorentzVectors of \f$p_1\f$ and \f$p_2\f$ and the jacobian associated with the change of variable. |
 *
 * \note This block has been partially validated and is probably safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */
class SecondaryBlockE: public Module {
    public:

        SecondaryBlockE(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()),
            sqrt_s(parameters.globalParameters().get<double>("energy")) {
                s12 = get<double>(parameters.get<InputTag>("s12"));
                s123 = get<double>(parameters.get<InputTag>("s123"));
                
                m_p1 = get<LorentzVector>(parameters.get<InputTag>("p1"));
                m_p2 = get<LorentzVector>(parameters.get<InputTag>("p2"));
                m_p3 = get<LorentzVector>(parameters.get<InputTag>("p3"));
            };

        virtual Status work() override {

            solutions->clear();

            // Don't spend time on unphysical part of phase-space
            if (*s12 > SQ(sqrt_s) || *s123 > SQ(sqrt_s) || *s12 > *s123)
               return Status::NEXT;

            const double m1 = m_p1->M();
            const double sq_m1 = SQ(m1);
            const double m2 = m_p2->M();
            const double m3 = m_p3->M();
            const double p3 = m_p3->P();
            const double E3 = m_p3->E();
            const double sq_E3 = SQ(E3);

            const double c12 = ROOT::Math::VectorUtil::CosTheta(*m_p1, *m_p2);
            const double c13 = ROOT::Math::VectorUtil::CosTheta(*m_p1, *m_p3);
            const double c23 = ROOT::Math::VectorUtil::CosTheta(*m_p2, *m_p3);

            double X = p3 * c23 - E3;
            double Y = *s123 - *s12 - SQ(m3);

            std::vector<double> abs_p1, abs_p2;
            solve2Quads(SQ(X), SQ(p3 * c13) - sq_E3, 2 * p3 * c13 * X,  X * Y, p3 * c13 * Y, 0.25 * SQ(Y) - sq_E3 * sq_m1,
                        2 * X / E3, 0, 2 * (p3 * c13 / E3 - c12), Y / E3, 0, sq_m1 + SQ(m2) - *s12,
                        abs_p2, abs_p1);

            // Use now the obtained |p1| and |p2| solutions to build p1 and p2 (m2=0!)
            for (std::size_t i = 0; i < abs_p1.size(); i++) {
                // Skip unphysical solutions
                if (abs_p1[i] <= 0 || abs_p2[i] <= 0)
                    continue;

                const double sin_theta_1 = std::sin(m_p1->Theta());
                const double sin_theta_2 = std::sin(m_p2->Theta());
                const double E1 = std::sqrt(SQ(abs_p1[i]) + sq_m1);

                LorentzVector p1_sol, p2_sol;
                p1_sol.SetPxPyPzE(
                        abs_p1[i] * std::cos(m_p1->Phi()) * sin_theta_1,
                        abs_p1[i] * std::sin(m_p1->Phi()) * sin_theta_1,
                        abs_p1[i] * std::cos(m_p1->Theta()),
                        E1); 
                p2_sol.SetPxPyPzE(
                        abs_p2[i] * std::cos(m_p2->Phi()) * sin_theta_2,
                        abs_p2[i] * std::sin(m_p2->Phi()) * sin_theta_2,
                        abs_p2[i] * std::cos(m_p2->Theta()),
                        abs_p2[i]);

                // Compute jacobian
                const double jacobian = abs_p2[i] * SQ(abs_p1[i]) * sin_theta_1 * sin_theta_2 / 
                                            (1024 * std::pow(M_PI, 6) * std::abs( 
                                                    abs_p2[i] * (abs_p1[i] - E1 * c12) * X + (E3 * abs_p1[i] - E1 * p3 * c13) * (E1 - abs_p1[i] * c12) )
                                            );

                Solution solution { { p1_sol, p2_sol }, jacobian, true };
                solutions->push_back(solution);
            }
            
            return (solutions->size() > 0) ? Status::OK : Status::NEXT;

        }

    private:
        double sqrt_s;

        // Inputs
        Value<double> s12;
        Value<double> s123;
        Value<LorentzVector> m_p1;
        Value<LorentzVector> m_p2;
        Value<LorentzVector> m_p3;

        // Output
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(SecondaryBlockE);
