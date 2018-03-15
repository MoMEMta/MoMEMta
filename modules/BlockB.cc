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
 * - \f$p_1^2 = m_1^2\f$
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
 *   | `m1` | double, default 0 | Mass of the invisible particle \f$p_1\f$. |
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `s12` | double | Squared invariant mass of the particle decaying into the missing particle (\f$p_1\f$) and the visible particle, \f$p_2\f$. Typically coming from a BreitWignerGenerator module.
 *   | `p2` | LorentzVector | LorentzVector of the particle \f$p_2\f$, as described above. |
 *   | `branches` | vector(LorentzVector) | LorentzVector of all the other particles in the event, taken into account when computing \f$\vec{p}_{T}^{tot}\f$ and checking if the solutions are physical. |
 *   | `met` | LorentzVector, default `met::p4` | LorentzVector of the MET |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embed  the LorentzVector of the invisible particle (\f$p_1\f$ in this case) and the associated jacobian. These solutions should be fed as input to the Looper module. |
 *
 * \note This block has been validated and is safe to use.
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

            m1 = parameters.get<double>("m1", 0.);

            s12 = get<double>(parameters.get<InputTag>("s12"));

            p2 = get<LorentzVector>(parameters.get<InputTag>("p2"));

            if (parameters.exists("branches")) {
                auto branches_tags = parameters.get<std::vector<InputTag>>("branches");
                for (auto& t: branches_tags)
                    m_branches.push_back(get<LorentzVector>(t));
            }

            // If the met input is specified, get it, otherwise retrieve default
            // one ("met::p4")
            InputTag met_tag;
            if (parameters.exists("met")) {
                met_tag = parameters.get<InputTag>("met");
            } else {
                met_tag = InputTag({"met", "p4"});
            }

            m_met = get<LorentzVector>(met_tag);
        };

        virtual Status work() override {

            solutions->clear();

            // Equations to solve:
            //(1) (p1 + p2)^2 = s12 = M1^2 + M2^2 + 2 E1 E2 - 2 p1x p2x - 2 p1y p2y - 2 p1z p2z
            //(2)  p1x = - pTx  #Coming from pT neutrino = -pT visible = - (p2 + ISR)
            //(3)  p1y = - pTy  #Coming from pT neutrino = -pT visible = - (p2 + ISR)
            //(4)  E1^2 - p1x^2 - p1y^2 - p1z^2 = M1^2

            const double p11 = SQ(m1);
            const double p22 = p2->M2();

            // Don't spend time on unphysical part of phase-space
            if (*s12 >= SQ(sqrt_s) || *s12 <= p11 + p22)
                return Status::NEXT;

            LorentzVector pT;
            if (pT_is_met) {
                pT = - *m_met;
            } else {
                pT = *p2;
                for (size_t i = 0; i < m_branches.size(); i++) {
                    pT += *m_branches[i];
                }
            }

            // From eq.(1) p1z = B*E1 + A
            // From eq.(4) + eq.(1) (1 - B^2) E1^2 - 2 A B E1 + C - A^2 - M1^2 = 0

            const double A = - (*s12 - p11 - p22 - 2 * (pT.Px() * p2->Px() + pT.Py() * p2->Py())) / (2 * p2->Pz());
            const double B = p2->E() / p2->Pz();
            const double C = - SQ(pT.Px()) - SQ(pT.Py());

            // Solve quadratic a*E1^2 + b*E1 + c = 0
            const double a = 1 - SQ(B);
            const double b = - 2 * A * B;
            const double c = C - SQ(A) - p11;

            std::vector<double> E1;

            solveQuadratic(a, b, c, E1, false);

            if (E1.size() == 0)
                return Status::NEXT;

            for (unsigned int i = 0; i < E1.size(); i++){
                const double e1 = E1.at(i);

                if (e1 <= 0) continue;

                LorentzVector p1(-pT.Px(), -pT.Py(), A + B*e1, e1);

                // Check if solutions are physical
                LorentzVector tot = p1 + *p2;
                for (size_t i = 0; i < m_branches.size(); i++)
                    tot += *m_branches[i];
                double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
                double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;
                if (q1Pz > sqrt_s / 2 || q2Pz > sqrt_s / 2)
                    continue;

                if (!ApproxComparison(p1.M() / p1.E(), m1 / p1.E())) {
#ifndef NDEBUG
                    LOG(trace) << "[BlockB] Throwing solution because of invalid mass. " <<
                               "Expected " << m1 << ", got " << p1.M();
#endif
                    continue;
                }

                if (!ApproxComparison((p1 + *p2).M2(), *s12)) {
#ifndef NDEBUG
                    LOG(trace) << "[BlockB] Throwing solution because of invalid invariant mass. " <<
                               "Expected " << *s12 << ", got " << (p1 + *p2).M2();
#endif
                    continue;
                }

                const double inv_jacobian = SQ(sqrt_s) * std::abs(p2->Pz() * e1 - p2->E() * p1.Pz());

                Solution s { {p1}, M_PI / inv_jacobian, true };
                solutions->push_back(s);
            }

            return solutions->size() > 0 ? Status::OK : Status::NEXT;
        }

    private:
        double sqrt_s;
        bool pT_is_met;
        double m1;

        // Inputs
        Value<double> s12;
        Value<LorentzVector> p2;
        std::vector<Value<LorentzVector>> m_branches;
        Value<LorentzVector> m_met;

        // Outputs
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};

REGISTER_MODULE(BlockB)
        .Input("s12")
        .Input("p2")
        .OptionalInputs("branches")
        .Input("met=met::p4")
        .Output("solutions")
        .GlobalAttr("energy:double")
        .Attr("pT_is_met:bool=false")
        .Attr("m1:double=0.");
