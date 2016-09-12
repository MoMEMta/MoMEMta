
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

/** \brief \f$\require{cancel}\f$ Final (main) Block A, describing \f$q_1 q_2 \to p_1 + p_2 + X\f$
 *
 * \f$q_1\f$ and \f$q_2\f$ are Bjorken fractions, \f$p_1\f$ and \f$p_2\f$ are the 4-momenta of the visible
 * particles produced in the process.
 *
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrization to the \f$\frac{1}{16\pi^{2} E_1 E_2} d \theta_1 d \theta_2 d \phi_1 d \phi_2 \times J\f$ parametrization,
 * where \f$ \theta_1 \f$ and \f$ \theta_2 \f$ are the polar angles and \f$ \phi_1 \f$ and \f$ \phi_2 \f$ are the azimuthal angles of
 * the particles labeled with 1 and 2.
 * 
 * The balanced modules \f$|p_1|\f$ and \f$|p_2|\f$ of the visible particles are computed based on the following set of equations:
 *
 * - \f$|p_1| sin \theta_1 cos \phi_1 + |p_2| sin \theta_2 cos \phi_2 = -p_{x}^{branches}\f$
 * - \f$|p_1| sin \theta_1 sin \phi_1 + |p_2| sin \theta_2 sin \phi_2 = -p_{y}^{branches}\f$
 * i.e. balance of the transverse momentum \f$ p_{T}^{branches} of the particles present in the process.
 *
 * Only one solution (\f$|p_1|, |p_2|\f$)  is possible.
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
 *   | `inputs` | vector(LorentzVector) | LorentzVector of all the experimentally reconstructed particles. Only the first two particles are used explicitly by the block, but there can be other visible objects in the event, taken into account when computing \f$\vec{p}_{T}^{branches}\f$. |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embeds the LorentzVectors of the balanced visible particles (ie. one \f$(p_1, p_2)\f$ pair) and the associated jacobian. |
 * \warning This block is **not** validated for the moment. The output is maybe correct, maybe not. Use with caution.
 *
 * \ingroup modules
 */

class BlockA: public Module {
    public:
  
        BlockA(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {

            sqrt_s = parameters.globalParameters().get<double>("energy");
            
            auto particle_tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& t: particle_tags)
                m_particles.push_back(get<LorentzVector>(t));
        };
 
        virtual Status work() override {

            solutions->clear();

            const LorentzVector& p1 = *m_particles[0];
            const LorentzVector& p2 = *m_particles[1];

            LorentzVector pb;
            for (size_t i = 2; i < m_particles.size(); i++) {
                pb += *m_particles[i];
            }

            double pbx = pb.Px();
            double pby = pb.Py();
            const double theta1 = p1.Theta();
            const double phi1 = p1.Phi();
            const double theta2 = p2.Theta();
            const double phi2 = p2.Phi();
            const double m1 = p1.M();
            const double m2 = p2.M();

            // pT = p1+p2+pb = 0. Equivalent to the following system:
            // p1x+p2x = -pbx
            // p1y+p2y = -pby,
            // where: p1x=modp1*sin(theta1)*cos(phi1), p1y=modp1*sin(theta1)*sin(phi1),
            //        p2x=modp2*sin(theta2)*cos(phi2), p2y=modp2*sin(theta2)*sin(phi2)
            // Get modp1, modp2 as solutions of this system

            std::vector<double> modp1;
            std::vector<double> modp2;

            const double sin_theta1 = std::sin(theta1);
            const double cos_phi1 = std::cos(phi1);
            const double sin_theta2 = std::sin(theta2);
            const double cos_phi2 = std::cos(phi2);
            const double sin_phi1 = std::sin(phi1);
            const double sin_phi2 = std::sin(phi2);
            const double cos_theta1 = std::cos(theta1);
            const double cos_theta2 = std::cos(theta2);

            const double a10 = sin_theta1 * cos_phi1;
            const double a01 = sin_theta2 * cos_phi2;
            const double a00 = pbx;
            const double b10 = sin_theta1 * sin_phi1;
            const double b01 = sin_theta2 * sin_phi2;
            const double b00 = pby;

            bool foundSolution = solve2Linear(a10, a01, a00, b10, b01, b00, modp1, modp2, false);

            if (!foundSolution)
               return Status::NEXT;

            double mod_p1 = modp1[0];
            double mod_p2 = modp2[0];
            if (mod_p1 < 0 || mod_p2 < 0)
               return Status::NEXT;

            double E1 = sqrt(SQ(mod_p1) + SQ(m1));
            double E2 = sqrt(SQ(mod_p2) + SQ(m2));

            LorentzVector gen_p1(mod_p1 * sin_theta1 * cos_phi1, mod_p1 * sin_theta1 * sin_phi1, mod_p1 * cos_theta1, E1);
            LorentzVector gen_p2(mod_p2 * sin_theta2 * cos_phi2, mod_p2 * sin_theta2 * sin_phi2, mod_p2 * cos_theta2, E2);

            double jacobian = (SQ(mod_p1) * SQ(mod_p2)) / (8 * SQ(M_PI * sqrt_s) * E1 * E2);
            jacobian *= 1. / std::abs(cos_phi1 * sin_phi2 - sin_phi1 * cos_phi2);

            Solution s = { {gen_p1, gen_p2}, jacobian, true };
            solutions->push_back(s);

            return Status::OK;
        }


    private:
        double sqrt_s;

        // Inputs
        std::vector<Value<LorentzVector>> m_particles;
        // Outputs
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(BlockA);
