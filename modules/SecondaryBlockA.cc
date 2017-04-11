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

/** \brief \f$\require{cancel}\f$ Secondary Block A, describing \f$s_{1234} \to ( s_{123} \to s_{12}(\to \cancel{p_1} + p_2) + p_3 ) + p_4\f$
 *
 * This Secondary Block reconstructs \f$p_1\f$ using:
 * - \f$p_1\f$'s mass
 * - \f$p_2\f$, \f$p_3\f$ and \f$p_4\f$
 * - \f$s_{12}\f$, \f$s_{123}\f$ and \f$s_{1234}\f$
 *
 * by solving these equations, for which there are up to two solutions:
 *  - \f$s_{12} = (p_1 + p_2)^2\f$
 *  - \f$s_{123} = (p_1 + p_2 + p_3)^2\f$
 *  - \f$s_{1234} = (p_1 + p_2 + p_3 + p_4)^2\f$
 *  - \f$p_1^2 = m_1^2\f$
 *
 * The standard phase-space parametrization associated with this block reads: \f$ \frac{d^3 p_1}{(2\pi)^3 2E_1}\f$. The procedure implies a change of variable leading to the following phase-space parametrization: \f$ \frac{1}{(2\pi)^3 2 E_1} ds_{12} ds_{123} ds_{1234} \times J \f$ (where \f$J\f$ is the jacobian associated with the transformation).
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
 *   | `s12` <br/> `s123` <br/> `s1234` | double | Squared invariant mass of the propagators (GeV\f$^2\f$). |
 *   | `p1` | LorentzVector | LorentzVector of the particle we want to reconstruct. This input will be used only to fix the mass of \f$p_1\f$. |
 *   | `p2` <br/> `p3` <br/> `p4` | LorentzVector | LorentzVector of the decay products used to reconstruct \f$p_1\f$ (see above description).|
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the above equations. Each solution embed the parton level LorentzVector of \f$p_1\f$ and the jacobian associated with the change of variable. |
 *
 * \note This block has been partially validated and is probably safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */
class SecondaryBlockA: public Module {
    public:

        SecondaryBlockA(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()),
            sqrt_s(parameters.globalParameters().get<double>("energy")) {
                s12 = get<double>(parameters.get<InputTag>("s12"));
                s123 = get<double>(parameters.get<InputTag>("s123"));
                s1234 = get<double>(parameters.get<InputTag>("s1234"));
                
                m_p1 = get<LorentzVector>(parameters.get<InputTag>("p1"));
                m_p2 = get<LorentzVector>(parameters.get<InputTag>("p2"));
                m_p3 = get<LorentzVector>(parameters.get<InputTag>("p3"));
                m_p4 = get<LorentzVector>(parameters.get<InputTag>("p4"));
            };

        virtual Status work() override {

            solutions->clear();

            // Don't spend time on unphysical part of phase-space
            if (*s12 > SQ(sqrt_s) || *s123 > SQ(sqrt_s) || *s1234 > SQ(sqrt_s) || *s12 > *s123 || *s123 > *s1234 )
               return Status::NEXT;

            // p1: retrieve the desired mass of the missing particle
            const double m1 = m_p1->M();
            const double sq_m1 = SQ(m1);

            // Store things we might need more than once
            const double p2x = m_p2->Px();
            const double p2y = m_p2->Py();
            const double p2z = m_p2->Pz();
            const double E2 = m_p2->E();
            const double m2 = m_p2->M();
            const double sq_m2 = SQ(m2);

            const double p3x = m_p3->Px();
            const double p3y = m_p3->Py();
            const double p3z = m_p3->Pz();
            const double E3 = m_p3->E();
            const double m3 = m_p3->M();
            const double sq_m3 = SQ(m3);

            const double p4x = m_p4->Px();
            const double p4y = m_p4->Py();
            const double p4z = m_p4->Pz();
            const double E4 = m_p4->E();
            const double m4 = m_p4->M();
            const double sq_m4 = SQ(m4);

            const double p2p3 = m_p2->Dot(*m_p3);
            const double p2p4 = m_p2->Dot(*m_p4);
            const double p3p4 = m_p3->Dot(*m_p4);

            /* Analytically solve the system:
             * ai1 p1x + ai2 p1y + ai3 p1z = bi E1 + ci, with i = 1...3
             * 
             * This gives p1 as a function of E1:
             * p1x = Ax E1 + Bx
             * p1y = Ay E1 + By
             * p1z = Az E1 + Bz
             */
            const double a11 = p2x;
            const double a12 = p2y;
            const double a13 = p2z;

            const double a21 = p2x + p3x;
            const double a22 = p2y + p3y;
            const double a23 = p2z + p3z;

            const double a31 = p2x + p3x + p4x;
            const double a32 = p2y + p3y + p4y;
            const double a33 = p2z + p3z + p4z;

            const double b1 = E2;
            const double c1 = 0.5 * (sq_m1 + sq_m2 - *s12);
            const double b2 = E2 + E3;
            const double c2 = 0.5 * (sq_m1 + sq_m2 + sq_m3 - *s123) + p2p3;
            const double b3 = E2 + E3 + E4;
            const double c3 = 0.5 * (sq_m1 + sq_m2 + sq_m3 + sq_m4 - *s1234) + p2p3 + p3p4 + p2p4;

            const double det = (a13 * a22 - a12 * a23) * a31 - (a13 * a21 - a11 * a23) * a32 + (a12 * a21 - a11 * a22) * a33;

            const double Ax = ((a13 * a22 - a12 * a23) * b3 - (a13 * a32 - a12 * a33) * b2 + (a23 * a32 - a22 * a33) * b1) / det;
            const double Bx = ((a13 * a22 - a12 * a23) * c3 - (a13 * a32 - a12 * a33) * c2 + (a23 * a32 - a22 * a33) * c1) / det;

            const double Ay = - ((a13 * a21 - a11 * a23) * b3 - (a13 * a31 - a11 * a33) * b2 + (a23 * a31 - a21 * a33) * b1) / det;
            const double By = - ((a13 * a21 - a11 * a23) * c3 - (a13 * a31 - a11 * a33) * c2 + (a23 * a31 - a21 * a33) * c1) / det;

            const double Az = ((a12 * a21 - a11 * a22) * b3 - (a12 * a31 - a11 * a32) * b2  + (a22 * a31 - a21 * a32) * b1) / det;
            const double Bz = ((a12 * a21 - a11 * a22) * c3 - (a12 * a31 - a11 * a32) * c2  + (a22 * a31 - a21 * a32) * c1) / det;

            // Now the mass-shell condition for p1 gives a quadratic equation in E1 with up to two solutions
            std::vector<double> E1_sol;
            bool foundSolution = solveQuadratic(SQ(Ax) + SQ(Ay) + SQ(Az) - 1, 2 * (Ax * Bx + Ay * By + Az * Bz), SQ(Bx) + SQ(By) + SQ(Bz) + sq_m1, E1_sol);

            if (!foundSolution)
                return Status::NEXT;

            // Use now the obtained solutions of E1 to build the full p1
            for (const double E1: E1_sol) {
                // Skip unphysical solutions
                if (E1 <= 0)
                    continue;

                const double p1x = Ax * E1 + Bx;
                const double p1y = Ay * E1 + By;
                const double p1z = Az * E1 + Bz;

                LorentzVector p1_sol(p1x, p1y, p1z, E1);

                // Compute jacobian
                const double jacobian = 1. / (128 * std::pow(M_PI, 3) * std::abs( 
                                            E4 * (p1z*p2y*p3x - p1y*p2z*p3x - p1z*p2x*p3y + p1x*p2z*p3y + p1y*p2x*p3z - p1x*p2y*p3z)
                                            + E2*p1z*p3y*p4x - E1*p2z*p3y*p4x - E2*p1y*p3z*p4x + E1*p2y*p3z*p4x - E2*p1z*p3x*p4y + E1*p2z*p3x*p4y + 
                                            E2*p1x*p3z*p4y - E1*p2x*p3z*p4y + 
                                            (E2*p1y*p3x - E1*p2y*p3x - E2*p1x*p3y + E1*p2x*p3y) * p4z + 
                                            E3 * (-(p1z*p2y*p4x) + p1y*p2z*p4x + p1z*p2x*p4y - p1x*p2z*p4y - p1y*p2x*p4z + p1x*p2y*p4z)
                                    ));

                Solution solution { { p1_sol }, jacobian, true };
                solutions->push_back(solution);
            }
            
            return (solutions->size() > 0) ? Status::OK : Status::NEXT;

        }

    private:
        double sqrt_s;

        // Inputs
        Value<double> s12;
        Value<double> s123;
        Value<double> s1234;
        Value<LorentzVector> m_p1;
        Value<LorentzVector> m_p2;
        Value<LorentzVector> m_p3;
        Value<LorentzVector> m_p4;

        // Output
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(SecondaryBlockA);
