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
#include <momemta/Types.h>
#include <momemta/Math.h>


/** \brief \f$\require{cancel}\f$ Final (main) Block B, describing \f$q_1 q_2 \to X + s_{12} (\to \cancel{p_1} p_2)\f$
 *
 * \f$q_1\f$ and \f$q_2\f$ are Bjorken fractions, and \f$s_{12}\f$ is a particle decaying
 * into \f$p_1\f$ (invisible particle) and \f$p_2\f$ (visible particle).
 *
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrization to the \f$\frac{1}{4\pi E_1} ds_{12} \times J\f$ parametrization.
 *
 * The integration is performed over \f$s_{12}\f$ with \f$p_2\f$ as input. Per integration point,
 * the LorentzVector of the invisible particle, \f$p_1\f$, is computed based on the following set
 * of equations:
 *
 * - \f$s_{12} = (p_1 + p_2)^2\f$
 * - Conservation of momentum (with \f$\vec{p}_T^{tot}\f$ the total transverse momentum of visible particles):
 *  - \f$p_{1x} = - p_{Tx}^{tot}\f$
 *  - \f$p_{1y} = - p_{Ty}^{tot}\f$
 * - \f$p_1^2 = m_1^2 = 0\f$ (FIXME)
 *
 * Up to two \f$p_1\f$ solutions are possible.
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
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `pT_is_met` | bool, default false | Fix \f$\vec{p}_{T}^{tot} = -\vec{\cancel{E_T}}\f$ or \f$\vec{p}_{T}^{tot} = \sum_{i \in \text{ vis}} \vec{p}_i\f$ |
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `s12` | double | Invariant mass of the particle decaying into the missing particle (\f$p_1\f$) and the visible particle, \f$p_2\f$. Typically coming from a BreitWignerGenerator module.
 *   | `inputs` | vector(LorentzVector) | LorentzVector of all the experimentally reconstructed particles. In this Block there is only one visible particle used explicitly, \f$p_2\f$, but there can be other visible objects in the the event, taken into account when computing \f$\vec{p}_{T}^{tot}\f$.
 *   | `met` | LorentzVector, default `input::met` | LorentzVector of the MET |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embed  the LorentzVector of the invisible particle (\f$p_1\f$ in this case) and the associated jacobian. These solutions should be fed as input to the Looper module. |
 *
 * \warning This block is **not** validated for the moment. The output is maybe correct, maybe not. Use with caution.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */

class BlockB: public Module {
    public:

        BlockB(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {

            sqrt_s = parameters.globalParameters().get<double>("energy");
            pT_is_met = parameters.get<bool>("pT_is_met", false);

            s12 = get<double>(parameters.get<InputTag>("s12"));

            auto particle_tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& t: particle_tags)
                m_particles.push_back(get<LorentzVector>(t));

            // If the met input is specified, get it, otherwise retrieve default
            // one ("input::met")
            InputTag met_tag;
            if (parameters.exists("met")) {
                met_tag = parameters.get<InputTag>("met");
            } else {
                met_tag = InputTag({"input", "met"});
            }

            m_met = get<LorentzVector>(met_tag);
        };

        virtual Status work() override {

            solutions->clear();

            // Equations to solve:
            //(1) (p1 + p2)^2 = s12 = M1^2 + M2^2 + 2E1E2 + 2p1xp2x + 2p1yp2y + p1zp2z
            //(2)  p1x = - pTx  #Coming from pT neutrino = -pT visible = - (p2 + ISR)
            //(3)  p1y = - pTy  #Coming from pT neutrino = -pT visible = - (p2 + ISR)
            //(4)  M1 = 0 -> E1^2 = p1x^2 + p1y^2 + p1z^2

            // Don't spend time on unphysical part of phase-space
            if (*s12 > SQ(sqrt_s))
                return Status::NEXT;

            const LorentzVector& p2 = *m_particles[0];

            LorentzVector pT;
            if (pT_is_met) {
                pT = - *m_met;
            } else {
                pT = p2;
                for (size_t i = 1; i < m_particles.size(); i++) {
                    pT += *m_particles[i];
                }
            }

            const double p22 = p2.M2();

            // From eq.(1) p1z = -B*E1 + A
            // From eq.(4) + eq.(1) (1-B^2)* E1^2 + 2AB* E1 - C = 0

            const double A = (*s12 - p22 + 2*(pT.Px()*p2.Px() + pT.Py()*p2.Py()))/(2*p2.Pz());
            const double B = p2.E()/p2.Pz();
            const double C = SQ(pT.Px()) + SQ(pT.Py());

            // Solve quadratic a*E1^2 + b*E1 + c = 0
            const double a = 1 - SQ(B);
            const double b = 2*A*B;
            const double c = -C;

            std::vector<double> E1;

            solveQuadratic(a, b, c, E1, false);

            if (E1.size() == 0)
                return Status::NEXT;

            for(unsigned int i=0; i<E1.size(); i++){
                const double e1 = E1.at(i);

                if (e1 < 0.) continue;

                LorentzVector p1(-pT.Px(), -pT.Py(), A - B*e1, e1);

                // Check if solutions are physical
                LorentzVector tot = p1;
                for (size_t i = 0; i < m_particles.size(); i++) {
                    tot += *m_particles[i];
                }
                double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
                double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;
                if(q1Pz > sqrt_s/2 || q2Pz > sqrt_s/2)
                    continue;

                Solution s { {p1}, computeJacobian(p1, p2), true };
                solutions->push_back(s);
            }

            return solutions->size() > 0 ? Status::OK : Status::NEXT;
        }

        double computeJacobian(const LorentzVector& p1, const LorentzVector& p2) {

            const double E1  = p1.E();
            const double p1z = p1.Pz();

            const double E2  = p2.E();
            const double p2z = p2.Pz();

            // Some extra info in MadWeight Source/MadWeight/blocks/class_b.f Good luck!!

            double inv_jac = 4.*SQ(M_PI*sqrt_s)*( p2z*E1 - E2*p1z);

            return 1. / std::abs(inv_jac);
        }

    private:
        double sqrt_s;
        bool pT_is_met;

        // Inputs
        Value<double> s12;
        std::vector<Value<LorentzVector>> m_particles;
        Value<LorentzVector> m_met;

        // Outputs
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(BlockB);
