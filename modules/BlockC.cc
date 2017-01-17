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

#include <momemta/Math.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Solution.h>
#include <momemta/Types.h>

/** \brief \f$\require{cancel}\f$ Final (main) Block C, describing \f$s_{123} (\to p_3 + s_{12} (\to \cancel{p_1} p_2))\f$
 *
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrisation for \f$p_{1 \dots 3} \times \delta^4\f$ to a parametrisation in terms of the two (squared) masses
 * of the intermediate propagators and the angular variables of \f$p3\f$.
 *
 * The integration is performed over \f$s_{12}, s_{123}\, \theta_3, \phi_3\f$ with \f$p_{1 \dots 3}\f$ as input. Per integration point,
 * the LorentzVector of the invisible particle, \f$p_1\f$, is computed as well as the energy of the (necessariy massless) particle \f$p3\f$, based on the following set
 * of equations:
 *
 * - \f$s_{12} = (p_1 + p_2)^2\f$
 * - \f$s_{123} = (p_1 + p_2 + p_3)^2\f$
 * - Conservation of momentum (with \f$\vec{p}_T^{branches}\f$ the total transverse momentum of all branches represented):
 *  - \f$p_{1x} + E_3 \sin\theta_3\cos\phi_3 = - p_{Tx}^{branches}\f$
 *  - \f$p_{1y} + E_3 \sin\theta_3\cos\phi_3 = - p_{Ty}^{branches}\f$
 *
 * Up to four solutions are possible for \f$(E1, \alpha)\f$, where \f$\alpha = 2p_1\dotp_2\f$.
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
 *   | `m1`  | double, default 0 | Mass of the invisible particle | 
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `s12` <br/> `s123` <br/>  | double | Squared invariant masses of the propagators. Typically coming from a BreitWignerGenerator or NarrowWidthApproximation module. |
 *   | `p2` <br/> `p3` | LorentzVector | The LorentzVector of \f$p2\f$ and the angles and mass of \f$p3\f$ will be used to reconstruct the event according to the above method. |
 *   | `branches` | vector(LorentzVector) | LorentzVectors of all the other particles in the event, taken into account when computing \f$\vec{p}_{T}^{branches}\f$ (if MET is not used), and checking if the solutions are physical. |
 *   | `met` | LorentzVector, default `met::p4` | LorentzVector of the MET |
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embeds the LorentzVectors of the invisible particle (ie. \f$(p_1)\f$) and the massless particle (ie. \f$(p_3)\f$) and the associated jacobian. These solutions should be fed as input to the Looper module. |
 *
 * \note This block has NOT been validated and is NOT safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */

class BlockC : public Module {
public:
    BlockC(PoolPtr pool, const ParameterSet& parameters) : Module(pool, parameters.getModuleName()) {
        sqrt_s = parameters.globalParameters().get<double>("energy");
        pT_is_met = parameters.get<bool>("pT_is_met", false);

        s12 = get<double>(parameters.get<InputTag>("s12"));
        s123 = get<double>(parameters.get<InputTag>("s123"));

        m1 = parameters.get<double>("m1", 0.);

        p2 = get<LorentzVector>(parameters.get<InputTag>("p2"));
        p3 = get<LorentzVector>(parameters.get<InputTag>("p3"));

        if (parameters.exists("branches")) {
            auto branches_tags = parameters.get<std::vector<InputTag>>("branches");
            for (auto& t : branches_tags)
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

        // Don't spend time on unphysical corner of the phase-space
        if (*s12 >= *s123 || *s12 >= SQ(sqrt_s) || *s123 >= SQ(sqrt_s))
            return Status::NEXT;

        // pT will be used to fix the transverse momentum of the reconstructed neutrinos
        // We can either enforce momentum conservation by disregarding the MET, ie:
        //  pT = sum of all the visible particles,
        // Or we can fix it using the MET given as input:
        //  pT = -MET
        // In the latter case, it is the user's job to ensure momentum conservation at
        // the matrix element level (by using the Boost module, for instance).
        LorentzVector pT;
        if (pT_is_met) {
            pT = -*m_met;
        } else {
            pT = *p2;
            for (size_t i = 0; i < m_branches.size(); i++) {
                pT += *m_branches[i];
            }
        }

        // p1x = alpha1 E1 + beta1 ALPHA + gamma1
        // p1y = ...(2)
        // p1z = ...(3)
        // E3  = ...(4)
        const double cosphi3 = std::cos(p3->Phi());
        const double sinphi3 = std::sin(p3->Phi());
        const double costhe3 = std::cos(p3->Theta());
        const double sinthe3 = std::sin(p3->Theta());

        const double p2Sq = p2->M2();
        const double E2 = p2->E();
        const double p2x = p2->Px();
        const double p2y = p2->Py();
        const double p2z = p2->Pz();

        const double pTx = pT.Px();
        const double pTy = pT.Py();

        // Term appears regularly, compute once.
        const double X = p2x * sinthe3 * cosphi3 + p2y * sinthe3 * sinphi3 + p2z * costhe3;

        // Denominator that appears in several of the follwing eq.
        // No need to compute it multiple times
        const double denom = 2. * (E2 - X);

        const double beta1 = (cosphi3 * sinthe3) / denom;
        const double gamma1 =
                -(2 * E2 * pTx - 2 * X * pTx - *s12 * cosphi3 * sinthe3 + *s123 * cosphi3 * sinthe3) / denom;

        const double beta2 = (sinthe3 * sinphi3) / denom;
        const double gamma2 =
                -(2 * E2 * pTy - 2 * X * pTy - *s12 * sinthe3 * sinphi3 + *s123 * sinthe3 * sinphi3) / denom;

        const double alpha3 = E2 / p2z;
        const double beta3 = (p2x * cosphi3 * sinthe3 + p2y * sinthe3 * sinphi3) / (-p2z * denom);
        const double gamma3 = 0.5 *
                              (-*s12 + SQ(m1) + p2Sq + 2 * p2x * (pTx + sinthe3 * cosphi3 * (*s123 - *s12) / denom) +
                               2 * p2y * (pTy + sinthe3 * sinphi3 * (*s123 - *s12) / denom)) /
                              p2z;

        const double beta4 = -1. / denom;
        const double gamma4 = (*s123 - *s12) / denom;

        // a11 E1^2 + a22 ALPHA^2 + a12 E1*ALPHA + a10 E1 + a01 ALPHA + a00 = 0
        // id. with bij
        const double a11 = SQ(alpha3) - 1;
        const double a22 = SQ(beta1) + SQ(beta2) + SQ(beta3);
        const double a12 = 2. * (alpha3 * beta3);
        const double a10 = 2. * (alpha3 * gamma3);
        const double a01 = 2. * (beta1 * gamma1 + beta2 * gamma2 + beta3 * gamma3);
        const double a00 = SQ(gamma1) + SQ(gamma2) + SQ(gamma3) + SQ(m1);

        const double b11 = 0;
        const double b22 = beta4 * (-beta1 * sinthe3 * cosphi3 - beta2 * sinthe3 * sinphi3 - beta3 * costhe3);
        const double b12 = beta4 - alpha3 * beta4 * costhe3;
        const double b10 = gamma4 - alpha3 * gamma4 * costhe3;
        const double b01 = -0.5 - (beta1 * gamma4 + beta4 * gamma1) * sinthe3 * cosphi3 -
                           (beta2 * gamma4 + beta4 * gamma2) * sinthe3 * sinphi3 -
                           (beta3 * gamma4 + beta4 * gamma3) * costhe3;
        const double b00 = gamma4 * (-gamma1 * sinthe3 * cosphi3 - gamma2 * sinthe3 * sinphi3 - gamma3 * costhe3);

        // Find the intersection of the 2 conics (at most 4 real solutions for (e1,ALPHA))
        std::vector<double> e1, ALPHA;
        solve2Quads(a11, a22, a12, a10, a01, a00, b11, b22, b12, b10, b01, b00, e1, ALPHA, false);

        // For each solution (e1,ALPHA), find the neutrino 4-momentum p1
        if (e1.size() == 0)
            return Status::NEXT;

        for (unsigned int i = 0; i < e1.size(); i++) {
            const double E1 = e1.at(i);
            const double alp = ALPHA.at(i);

            //Make sure E1 is not negative
            if (E1 < 0.)
                continue;

            const double E3 = beta4 * alp + gamma4;
            // Make sure E3 is not negative
            if (E3 < 0.)
                continue;

            const double p1x = beta1 * alp + gamma1;
            const double p1y = beta2 * alp + gamma2;
            const double p1z = alpha3 * E1 + beta3 * alp + gamma3;

            LorentzVector p1(p1x, p1y, p1z, E1);

            LorentzVector p3_sol(E3 * sinthe3 * cosphi3, E3 * sinthe3 * sinphi3, E3 * costhe3, E3);

            // Check if solutions are physical
            LorentzVector tot = pT + p1 + p3_sol;
            for (size_t i = 0; i < m_branches.size(); i++) {
                tot += *m_branches[i];
            }
            double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
            double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;
            if (q1Pz > sqrt_s / 2 || q2Pz > sqrt_s / 2)
                continue;

            const double A = cosphi3 * sinthe3 * (p1x * p2z - p1z * p2x + costhe3 * (E1 * p2x - E2 * p1x));
            const double B = sinphi3 * sinthe3 * (p1z * p2y + p1y * p2z);
            const double C = SQ(cosphi3) * SQ(sinthe3) * (E2 * p1z - E1 * p2z);
            const double D = SQ(sinphi3) * SQ(sinthe3) * (E2 * p1z - E1 * p2z);
            const double chi = 2 * (p3_sol.Dot(p1) + p3_sol.Dot(*p2)) / E3;

            double jacobian = E3 * sinthe3 / (16 * SQ(M_PI) * SQ(sqrt_s) *
                                              std::abs(chi * (E2 * p1z - E1 * p2z) + 2 * E2 * (A + B + C + D)));

            Solution s{{p1, p3_sol}, jacobian, true};
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
    Value<double> s123;
    std::vector<Value<LorentzVector>> m_branches;
    Value<LorentzVector> m_met;
    Value<LorentzVector> p2;
    Value<LorentzVector> p3;

    // Outputs
    std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(BlockC);
