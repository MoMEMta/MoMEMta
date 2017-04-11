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

/** \brief \f$\require{cancel}\f$ Secondary Block B, describing \f$s_{123} \to s_{12}(\to \cancel{p_1} + p_2) + p_3 \f$
 *
 * This Secondary Block determines \f$p_1^T\f$ and \f$p_1^z\f$ using
 * - \f$p_1\f$ phi angle and mass
 * - \f$p_2\f$
 * - \f$p_3\f$
 * - \f$s_{12}\f$
 * - \f$s_{123}\f$
 *
 * by solving the equations :
 *  - \f$s_{12} = (p_1 + p_2)^2\f$
 *  - \f$s_{123} = (p_1 + p_2 + p_3)^2\f$.
 *
 * The value of E1 is fixed using the mass-shell condition.
 *
 * The standard phase-space parametrization associated with this block reads: \f$ \frac{d^3p_1}{(2\pi)^32E_1}\f$. The procedure implies a change of variable leading to the following phase-space parametrization: \f$ \frac{1}{(2\pi)^32E_1}d\phi_1ds_{12}ds_{123}\times J \f$ with the jacobian \f$ J = \frac{E_1 |\boldsymbol{p_1}|^2 \sin\theta_1}{2|E_2|\boldsymbol{p_1}|-E_1|\boldsymbol{p_2}|\cos\theta_{12}|} \f$.
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
 *   | `p1` | LorentzVector | LorentzVector of the missing particle we want to reconstruct. It will be used only to retrieve the particle phi angle and mass.|
 *   | `p2` <br/> `p3` | LorentzVector | LorentzVectors of the two decay products used to reconstruct fully `p1` (see above description).|
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the equations \f$s_{12} = (p_1 + p_2)^2\f$ and \f$s_{123} = (p_1 + p_2 + p_3)^2\f$. Each solution embed the parton level LorentzVector of the missing product \f$p_1\f$ and the jacobian associated with the change of variable mentioned above.|
 *
 * \note This block has been partially validated and is probably safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */
class SecondaryBlockB: public Module {
    public:

        SecondaryBlockB(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()),
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

            // Solving the equations
            //      s_{12} = (p_1 + p_2)^2
            //      s_{123} = (p_1 + p_2 + p_3)^2
            // using \vec{p_i}.\vec{p_j} = p_i^T * p_j^T * cos(\Phi_{ij}) + p_i^z * p_j^z
            // allows to express p1t as p1t_linear * E1 + p1t_indep and p1z as p1z_linear * E1 + p1z_indep
            const double m1 = m_p1->M();
            const double m2 = m_p2->M();
            const double m3 = m_p3->M();
            const double m1_squared = SQ(m1);
            const double m2_squared = SQ(m2);
            const double m3_squared = SQ(m3);
            const double E2 = m_p2->E();
            const double E3 = m_p3->E();
            const double p2z = m_p2->Pz();
            const double p3z = m_p3->Pz();
            const double p2t = m_p2->Pt();
            const double p3t = m_p3->Pt();
            const double cosPhi12 = std::cos(ROOT::Math::VectorUtil::DeltaPhi(*m_p1, *m_p2));
            const double cosPhi13 = std::cos(ROOT::Math::VectorUtil::DeltaPhi(*m_p1, *m_p3));
            const double cosPhi23 = std::cos(ROOT::Math::VectorUtil::DeltaPhi(*m_p2, *m_p3));

            const double denominator = cosPhi13 * p2z * p3t - cosPhi12 * p2t * p3z;
            const double E2E3 = E2 * E3;
            const double p2zp3z = p2z * p3z;
            const double p2tp3t = p2t * p3t;
            const double E3p2z_E2p3z = E3 * p2z - E2 * p3z;
            const double p1t_linear = E3p2z_E2p3z / denominator;
            const double p1t_indep = (p2z * (2 * (E2E3 - p2zp3z - cosPhi23 * p2tp3t) + m3_squared - *s123 + *s12) - p3z * (m1_squared + m2_squared - *s12)) / (2. * denominator);

            const double p1z_linear = (cosPhi12 * E3 * p2t - cosPhi13 * E2 * p3t) / (-denominator);
            const double p1z_indep = (-(cosPhi13 * p3t * (m1_squared + m2_squared - *s12)) + cosPhi12 * p2t * (2 * (E2E3 - cosPhi23 * p2tp3t - p2zp3z) + m3_squared + *s12 - *s123)) / (2 * (-denominator));

            // Now, using E_1^2 - |\vec{p_1}|^2 = m_1^2
            // one can obtain E1.
            const double p1t_linear_squared = SQ(p1t_linear);
            const double p1t_indep_squared = SQ(p1t_indep);
            const double p1z_linear_squared = SQ(p1z_linear);
            const double p1z_indep_squared = SQ(p1z_indep);

            const double E1_indep = m1_squared + p1t_indep_squared + p1z_indep_squared;
            const double E1_linear = 2 * (p1t_indep * p1t_linear + p1z_indep * p1z_linear);
            const double E1_quadratic = -1 + p1t_linear_squared + p1z_linear_squared;

            std::vector<double> E1_solutions; // up to two solutions
            bool foundSolution = solveQuadratic(E1_quadratic, E1_linear, E1_indep, E1_solutions);
            if (!foundSolution)
                return Status::NEXT;

            // Use now the obtained E1 solutions to build p1
            for (const double& E1: E1_solutions) {
                // Skip unphysical solutions
                const double p1t = p1t_linear * E1 + p1t_indep;
                if (E1 <= m1 || p1t < 0)
                    continue;

                const double p1z = p1z_linear * E1 + p1z_indep;
                const double phi1 = m_p1->Phi();
                LorentzVector p1_sol;
                p1_sol.SetPxPyPzE(
                        p1t * std::cos(phi1),
                        p1t * std::sin(phi1),
                        p1z,
                        E1);
                // Compute jacobian
                const double jacobian = p1t / (64 * CB(M_PI) * std::abs(cosPhi12 * p2t * (E1 * p3z -  E3 * p1z) + cosPhi13 * p3t * (E2 * p1z  - E1 * p2z) + p1t * E3p2z_E2p3z));

                Solution solution { {p1_sol}, jacobian, true };
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
REGISTER_MODULE(SecondaryBlockB);
