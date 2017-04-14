
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

#include <momemta/Logging.h>
#include <momemta/ParameterSet.h>
#include <momemta/Module.h>
#include <momemta/Solution.h>
#include <momemta/Types.h>
#include <momemta/Utils.h>
#include <momemta/Math.h>

#include <Math/GenVector/VectorUtil.h>

/** \brief Final (main) Block G, describing \f$X + s_{12} (\to p_1 p_2) + s_{34} (\to p_3 p_4)\f$
 *
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrisation for \f$p_{1 \dots 4} \times \delta^4\f$ to a parametrisation in terms of the two (squared) masses of the intermediate propagators.
 *
 * In practice, this block allows to trade the energies of the four inputs \f$p_{1 \dots 4}\f$ for the two propagator invariants \f$s_{12}\f$ and \f$s_{34}\f$. The angles of the input particles are not affected.
 *
 * **Warning**: this change of variable is only valid if the four inputs \f$p_{1 \dots 4}\f$ are massless!
 *
 * The change of variable is done by solving the following system:
 * - \f$s_{12} = (p_1 + p_2)^2\f$
 * - \f$s_{34} = (p_3 + p_4)^2\f$
 * - Conservation of momentum (with \f$\vec{p}_T^{b}\f$ the total transverse momentum of possible other particles in the event):
 *  - \f$p_{1x} + p_{2x} + p_{3x} + p_{4x} = - p_{Tx}^{b}\f$
 *  - \f$p_{1y} + p_{2y} + p_{3y} + p_{4y} = - p_{Ty}^{b}\f$
 *
 * Up to four solutions are possible for \f$(p_1, p_2, p_3, p_4)\f$.
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
 *   | `s12` <br/> `s34` | double | Squared invariant masses of the propagators. Typically coming from a BreitWignerGenerator or NarrowWidthApproximation module.
 *   | `p1` ... `p4` | LorentzVector | LorentzVectors of the four particles in the event. The masses and angles of these particles will be used as input, and their energies modified according to the above method to reconstruct the event. |
 *   | `branches` | vector(LorentzVector) | LorentzVectors of all the other particles in the event, taken into account when computing \f$\vec{p}_{T}^{b}\f$. |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embeds the LorentzVectors of the particles whose energy was computed (ie. a set \f$(p_1, p_2, p_3, p_4)\f$) and the associated jacobian. |
 *
 * \note This block has been validated and is safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */

class BlockG: public Module {
    public:

        BlockG(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {

            sqrt_s = parameters.globalParameters().get<double>("energy");

            s12 = get<double>(parameters.get<InputTag>("s12"));
            s34 = get<double>(parameters.get<InputTag>("s34"));

            m_particles.push_back(get<LorentzVector>(parameters.get<InputTag>("p1")));
            m_particles.push_back(get<LorentzVector>(parameters.get<InputTag>("p2")));
            m_particles.push_back(get<LorentzVector>(parameters.get<InputTag>("p3")));
            m_particles.push_back(get<LorentzVector>(parameters.get<InputTag>("p4")));

            if (parameters.exists("branches")) {
                auto branches_tags = parameters.get<std::vector<InputTag>>("branches");
                for (auto& t: branches_tags)
                    m_branches.push_back(get<LorentzVector>(t));
            }
        };

        virtual Status work() override {

            solutions->clear();

            if (*s12 + *s34 >= SQ(sqrt_s))
                return Status::NEXT;

            const LorentzVector& p1 = *m_particles[0];
            const LorentzVector& p2 = *m_particles[1];
            const LorentzVector& p3 = *m_particles[2];
            const LorentzVector& p4 = *m_particles[3];

            LorentzVector pb;
            for (size_t i = 0; i < m_branches.size(); i++) {
                pb += *m_branches[i];
            }

            const double pbx = pb.Px();
            const double pby = pb.Py();
            const double sin_theta_1 = std::sin(p1.Theta());
            const double sin_theta_2 = std::sin(p2.Theta());
            const double sin_theta_3 = std::sin(p3.Theta());
            const double sin_theta_4 = std::sin(p4.Theta());
            const double phi_1 = p1.Phi();
            const double phi_2 = p2.Phi();
            const double phi_3 = p3.Phi();
            const double phi_4 = p4.Phi();
            const double sin_phi_3_2 = std::sin(phi_3 - phi_2);
            const double sin_phi_2_1 = std::sin(phi_2 - phi_1);
            const double sin_phi_4_2 = std::sin(phi_4 - phi_2);
            const double sin_phi_1_3 = std::sin(phi_1 - phi_3);
            const double sin_phi_1_4 = std::sin(phi_1 - phi_4);

            /*
             * p1 = alpha1 p3 + beta1 p4 + gamma1
             * p2 = alpha2 p3 + beta2 p4 + gamma2
             */

            const double denom_1 = sin_theta_1 * sin_phi_2_1;
            const double denom_2 = sin_theta_2 * sin_phi_2_1;

            const double alpha_1 = sin_theta_3 * sin_phi_3_2 / denom_1;
            const double beta_1 = sin_theta_4 * sin_phi_4_2 / denom_1;
            const double gamma_1 = ( std::cos(phi_2) * pby - std::sin(phi_2) * pbx ) / denom_1;

            const double alpha_2 = sin_theta_3 * sin_phi_1_3 / denom_2;
            const double beta_2 = sin_theta_4 * sin_phi_1_4 / denom_2;
            const double gamma_2 = ( std::sin(phi_1) * pbx - std::cos(phi_1) * pby ) / denom_2;

            const double cos_theta_34 = ROOT::Math::VectorUtil::CosTheta(p3, p4);
            const double cos_theta_12 = ROOT::Math::VectorUtil::CosTheta(p1, p2);
            const double X = 0.5 * (*s34) / (1 - cos_theta_34);
            const double Y = 0.5 * (*s12) / (1 - cos_theta_12);

            std::vector<double> gen_p3_solutions;
            solveQuartic(
                    alpha_1 * alpha_2,
                    alpha_1 * gamma_2 + gamma_1 * alpha_2,
                    gamma_1 * gamma_2 + (beta_1 * alpha_2 + alpha_1 * beta_2) * X - Y,
                    (beta_1 * gamma_2 + gamma_1 * beta_2) * X,
                    beta_1 * beta_2 * SQ(X),
                    gen_p3_solutions
                    );

            for (const auto& p3_sol: gen_p3_solutions) {

                const double p4_sol = X / p3_sol;
                const double p1_sol = alpha_1 * p3_sol + beta_1 * p4_sol + gamma_1;
                const double p2_sol = alpha_2 * p3_sol + beta_2 * p4_sol + gamma_2;

                if (p1_sol < 0 || p2_sol < 0 || p3_sol < 0 || p4_sol < 0)
                    continue;

                LorentzVector gen_p1(p1_sol * sin_theta_1 * std::cos(phi_1), p1_sol * sin_theta_1 * std::sin(phi_1), p1_sol * std::cos(p1.Theta()), p1_sol);
                LorentzVector gen_p2(p2_sol * sin_theta_2 * std::cos(phi_2), p2_sol * sin_theta_2 * std::sin(phi_2), p2_sol * std::cos(p2.Theta()), p2_sol);
                LorentzVector gen_p3(p3_sol * sin_theta_3 * std::cos(phi_3), p3_sol * sin_theta_3 * std::sin(phi_3), p3_sol * std::cos(p3.Theta()), p3_sol);
                LorentzVector gen_p4(p4_sol * sin_theta_4 * std::cos(phi_4), p4_sol * sin_theta_4 * std::sin(phi_4), p4_sol * std::cos(p4.Theta()), p4_sol);

                // Check if solutions are physical
                LorentzVector tot = gen_p1 + gen_p2 + gen_p3 + gen_p4 + pb;
                double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
                double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;
                if (q1Pz > sqrt_s / 2 || q2Pz > sqrt_s / 2)
                    continue;

                double jacobian = 1 / std::abs( 2 *
                        (1 - cos_theta_12) * (1 - cos_theta_34) * (
                            alpha_1 * gamma_2 * p3_sol + alpha_2  * p3_sol * (gamma_1 + 2 * alpha_1 * p3_sol) -
                            p4_sol * (beta_2 * gamma_1 + beta_1 * gamma_2 + 2 * beta_1 * beta_2 * p4_sol)
                        ) * SQ(sqrt_s) * sin_phi_2_1 );
                jacobian *= ( sin_theta_3 * sin_theta_4 * p1_sol * p2_sol * p3_sol * p4_sol ) / ( 16 * pow(2*M_PI, 8) );

                Solution s = { { gen_p1, gen_p2, gen_p3, gen_p4 }, jacobian, true };
                solutions->push_back(s);
            }

            if (!solutions->size())
               return Status::NEXT;

            return Status::OK;
        }


    private:
        double sqrt_s;

        // Inputs
        std::vector<Value<LorentzVector>> m_branches;
        std::vector<Value<LorentzVector>> m_particles;
        Value<double> s12, s34;
        // Outputs
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};

REGISTER_MODULE(BlockG)
        .Input("s12")
        .Input("s34")
        .Input("p1")
        .Input("p2")
        .Input("p3")
        .Input("p4")
        .OptionalInputs("branches")
        .Output("solutions")
        .GlobalAttr("energy:double");