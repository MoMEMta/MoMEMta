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

/** \brief \f$\require{cancel}\f$ Final (main) Block D, describing \f$X + s_{134} (\to p_4 + s_{13} (\to \cancel{p_1} p_3)) + s_{256} (\to p_6 + s_{25} (\to \cancel{p_2} p_5))\f$
 *
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrisation for \f$p_{1 \dots 6} \times \delta^4\f$ to a parametrisation in terms of the four (squared) masses
 * of the intermediate propagators.
 * 
 * The integration is performed over \f$s_{13}, s_{134}, s_{25}, s_{256}\f$ with \f$p_{3 \dots 6}\f$ as input. Per integration point, 
 * the LorentzVector of the invisible particles, \f$p_1\f$ and \f$p_2\f$, are computed based on the following set 
 * of equations:   
 *
 * - \f$s_{13} = (p_1 + p_3)^2\f$
 * - \f$s_{134} = (p_1 + p_3 + p_4)^2\f$
 * - \f$s_{25} = (p_2 + p_5)^2\f$
 * - \f$s_{256} = (p_2 + p_5 + p_6)^2\f$
 * - Conservation of momentum (with \f$\vec{p}_T^{tot}\f$ the total transverse momentum of visible particles):
 *  - \f$p_{1x} + p_{2x} = - p_{Tx}^{tot}\f$
 *  - \f$p_{1y} + p_{2y} = - p_{Ty}^{tot}\f$
 * - \f$p_1^2 = m_1^2 = 0\f$ (FIXME)
 * - \f$p_2^2 = m_2^2 = 0\f$ (FIXME)
 *
 * Up to four solutions are possible for \f$(p_1, p_2)\f$.
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
 *   | `s13` <br/> `s134` <br/> `s25` <br/> `s256` | double | Squared invariant masses of the propagators. Typically coming from a BreitWignerGenerator or NarrowWidthApproximation module.
 *   | `inputs` | vector(LorentzVector) | LorentzVectors of all the experimentally reconstructed particles. Only the first four are used explicitly by the block, but there can be other visible objects in the the event, taken into account when computing \f$\vec{p}_{T}^{tot}\f$ if needed.
 *   | `met` | LorentzVector, default `input::met` | LorentzVector of the MET |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embed  the LorentzVectors of the invisible particles (ie. one \f$(p_1, p_2)\f$ pair) and the associated jacobian. These solutions should be fed as input to the Looper module. |
 *
 * \note This block has been validated and is safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *   
 * \ingroup modules
 */

class BlockD: public Module {
    public:

        BlockD(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            sqrt_s = parameters.globalParameters().get<double>("energy");
            pT_is_met = parameters.get<bool>("pT_is_met", false);

            s13 = get<double>(parameters.get<InputTag>("s13"));
            s134 = get<double>(parameters.get<InputTag>("s134"));
            s25 = get<double>(parameters.get<InputTag>("s25"));
            s256 = get<double>(parameters.get<InputTag>("s256"));

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

            // Don't spend time on unphysical corner of the phase-space
            if (*s13 >= *s134 || *s25 >= *s256 || *s13 >= SQ(sqrt_s) || *s134 >= SQ(sqrt_s) || *s25 >= SQ(sqrt_s) || *s256 >= SQ(sqrt_s))
                return Status::NEXT;

            const LorentzVector& p3 = *m_particles[0];
            const LorentzVector& p4 = *m_particles[1];
            const LorentzVector& p5 = *m_particles[2];
            const LorentzVector& p6 = *m_particles[3];

            // pT will be used to fix the transverse momentum of the reconstructed neutrinos
            // We can either enforce momentum conservation by disregarding the MET, ie:
            //  pT = sum of all the visible particles, 
            // Or we can fix it using the MET given as input:
            //  pT = -MET
            // In the latter case, it is the user's job to ensure momentum conservation at
            // the matrix element level (by using the Boost module, for instance).
            
            LorentzVector pT;
            if (pT_is_met) {
                pT = - *m_met;
            } else {
                pT = p3 + p4 + p5 + p6;
                for (size_t i = 4; i < m_particles.size(); i++) {
                    pT += *m_particles[i];
                }
            }

            const double p34 = p3.Dot(p4);
            const double p56 = p5.Dot(p6);
            const double p33 = p3.M2();
            const double p44 = p4.M2();
            const double p55 = p5.M2();
            const double p66 = p6.M2();

            // A1 p1x + B1 p1y + C1 = 0, with C1(E1,E2)
            // A2 p1y + B2 p2y + C2 = 0, with C2(E1,E2)
            // ==> express p1x and p1y as functions of E1, E2

            const double A1 = 2.*( -p3.Px() + p3.Pz()*p4.Px()/p4.Pz() );
            const double A2 = 2.*( p5.Px() - p5.Pz()*p6.Px()/p6.Pz() );

            const double B1 = 2.*( -p3.Py() + p3.Pz()*p4.Py()/p4.Pz() );
            const double B2 = 2.*( p5.Py() - p5.Pz()*p6.Py()/p6.Pz() );

            const double Dx = B2*A1 - B1*A2;
            const double Dy = A2*B1 - A1*B2;

            const double X = 2*( pT.Px()*p5.Px() + pT.Py()*p5.Py() - p5.Pz()/p6.Pz()*( 0.5*(*s25 - *s256 + p66) + p56 + pT.Px()*p6.Px() + pT.Py()*p6.Py() ) ) + p55 - *s25;
            const double Y = p3.Pz()/p4.Pz()*( *s13 - *s134 + 2*p34 + p44 ) - p33 + *s13;

            // p1x = alpha1 E1 + beta1 E2 + gamma1
            // p1y = ...(2)
            // p1z = ...(3)
            // p2z = ...(4)
            // p2x = ...(5)
            // p2y = ...(6)

            const double alpha1 = -2*B2*(p3.E() - p4.E()*p3.Pz()/p4.Pz())/Dx;
            const double beta1 = 2*B1*(p5.E() - p6.E()*p5.Pz()/p6.Pz())/Dx;
            const double gamma1 = B1*X/Dx + B2*Y/Dx;

            const double alpha2 = -2*A2*(p3.E() - p4.E()*p3.Pz()/p4.Pz())/Dy;
            const double beta2 = 2*A1*(p5.E() - p6.E()*p5.Pz()/p6.Pz())/Dy;
            const double gamma2 = A1*X/Dy + A2*Y/Dy;

            const double alpha3 = (p4.E() - alpha1*p4.Px() - alpha2*p4.Py())/p4.Pz();
            const double beta3 = -(beta1*p4.Px() + beta2*p4.Py())/p4.Pz();
            const double gamma3 = ( 0.5*(*s13 - *s134 + p44) + p34 - gamma1*p4.Px() - gamma2*p4.Py() )/p4.Pz();

            const double alpha4 = (alpha1*p6.Px() + alpha2*p6.Py())/p6.Pz();
            const double beta4 = (p6.E() + beta1*p6.Px() + beta2*p6.Py())/p6.Pz();
            const double gamma4 = ( 0.5*(*s25 - *s256 + p66) + p56 + (gamma1 + pT.Px())*p6.Px() + (gamma2 + pT.Py())*p6.Py() )/p6.Pz();

            const double alpha5 = -alpha1;
            const double beta5 = -beta1;
            const double gamma5 = -pT.Px() - gamma1;

            const double alpha6 = -alpha2;
            const double beta6 = -beta2;
            const double gamma6 = -pT.Py() - gamma2;

            // a11 E1^2 + a22 E2^2 + a12 E1E2 + a10 E1 + a01 E2 + a00 = 0
            // id. with bij

            const double a11 = -1 + ( SQ(alpha1) + SQ(alpha2) + SQ(alpha3) );
            const double a22 = SQ(beta1) + SQ(beta2) + SQ(beta3);
            const double a12 = 2.*( alpha1*beta1 + alpha2*beta2 + alpha3*beta3 );
            const double a10 = 2.*( alpha1*gamma1 + alpha2*gamma2 + alpha3*gamma3 );
            const double a01 = 2.*( beta1*gamma1 + beta2*gamma2 + beta3*gamma3 );
            const double a00 = SQ(gamma1) + SQ(gamma2) + SQ(gamma3);

            const double b11 = SQ(alpha5) + SQ(alpha6) + SQ(alpha4);
            const double b22 = -1 + ( SQ(beta5) + SQ(beta6) + SQ(beta4) );
            const double b12 = 2.*( alpha5*beta5 + alpha6*beta6 + alpha4*beta4 );
            const double b10 = 2.*( alpha5*gamma5 + alpha6*gamma6 + alpha4*gamma4 );
            const double b01 = 2.*( beta5*gamma5 + beta6*gamma6 + beta4*gamma4 );
            const double b00 = SQ(gamma5) + SQ(gamma6) + SQ(gamma4);

            // Find the intersection of the 2 conics (at most 4 real solutions for (E1,E2))
            std::vector<double> E1, E2;
            solve2Quads(a11, a22, a12, a10, a01, a00, b11, b22, b12, b10, b01, b00, E1, E2, false);

            // For each solution (E1,E2), find the neutrino 4-momenta p1,p2

            if (E1.size() == 0)
                return Status::NEXT;

            for(unsigned int i=0; i<E1.size(); i++){
                const double e1 = E1.at(i);
                const double e2 = E2.at(i);

                if (e1 < 0. || e2 < 0.)
                    continue;

                LorentzVector p1(
                        alpha1*e1 + beta1*e2 + gamma1,
                        alpha2*e1 + beta2*e2 + gamma2,
                        alpha3*e1 + beta3*e2 + gamma3,
                        e1);

                LorentzVector p2(
                        alpha5*e1 + beta5*e2 + gamma5,
                        alpha6*e1 + beta6*e2 + gamma6,
                        alpha4*e1 + beta4*e2 + gamma4,
                        e2);

                // Check if solutions are physical
                LorentzVector tot = p1 + p2;
                for (size_t i = 0; i < m_particles.size(); i++) {
                    tot += *m_particles[i];
                }
                double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
                double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;
                if(q1Pz > sqrt_s/2 || q2Pz > sqrt_s/2)
                    continue;

                double jacobian = computeJacobian(p1, p2, p3, p4, p5, p6);
                Solution s { {p1, p2}, jacobian, true };
                solutions->push_back(s);
            }

            return solutions->size() > 0 ? Status::OK : Status::NEXT;
        }

        double computeJacobian(const LorentzVector& p1, const LorentzVector& p2, const LorentzVector& p3, const LorentzVector& p4, const LorentzVector& p5, const LorentzVector& p6) {

            const double E1  = p1.E();
            const double p1x = p1.Px();
            const double p1y = p1.Py();
            const double p1z = p1.Pz();

            const double E2  = p2.E();
            const double p2x = p2.Px();
            const double p2y = p2.Py();
            const double p2z = p2.Pz();

            const double E3  = p3.E();
            const double p3x = p3.Px();
            const double p3y = p3.Py();
            const double p3z = p3.Pz();

            const double E4  = p4.E();
            const double p4x = p4.Px();
            const double p4y = p4.Py();
            const double p4z = p4.Pz();

            const double E5  = p5.E();
            const double p5x = p5.Px();
            const double p5y = p5.Py();
            const double p5z = p5.Pz();

            const double E6  = p6.E();
            const double p6x = p6.Px();
            const double p6y = p6.Py();
            const double p6z = p6.Pz();

            const double E34  = E3 + E4;
            const double p34x = p3x + p4x;
            const double p34y = p3y + p4y;
            const double p34z = p3z + p4z;

            const double E56  = E5 + E6;
            const double p56x = p5x + p6x;
            const double p56y = p5y + p6y;
            const double p56z = p5z + p6z;

            // copied from Source/MadWeight/blocks/class_d.f

            double inv_jac = E3*(E5*
                    (p34z*(p1y*p2z*p56x - p1x*p2z*p56y - p1y*p2x*p56z + 
                           p1x*p2y*p56z) + 
                     p1z*(-(p2z*p34y*p56x) + p2z*p34x*p56y - 
                         p2y*p34x*p56z + p2x*p34y*p56z)) + 
                    (E56*p2z - E2*p56z)*
                    (p1z*p34y*p5x - p1y*p34z*p5x - p1z*p34x*p5y + 
                     p1x*p34z*p5y) + 
                    (E56*(p1z*p2y*p34x - p1z*p2x*p34y + p1y*p2x*p34z - 
                          p1x*p2y*p34z) + 
                     E2*(p1z*p34y*p56x - p1y*p34z*p56x - p1z*p34x*p56y + 
                         p1x*p34z*p56y))*p5z) + 
                E34*(E5*p2z*(p1z*p3y*p56x - p1y*p3z*p56x - p1z*p3x*p56y + 
                            p1x*p3z*p56y) + 
                        E5*(p1z*p2y*p3x - p1z*p2x*p3y + p1y*p2x*p3z - 
                            p1x*p2y*p3z)*p56z - 
                        (E56*p2z - E2*p56z)*
                        (p1z*p3y*p5x - p1y*p3z*p5x - p1z*p3x*p5y + p1x*p3z*p5y)
                        - (E56*(p1z*p2y*p3x - p1z*p2x*p3y + p1y*p2x*p3z - 
                                p1x*p2y*p3z) + 
                            E2*(p1z*p3y*p56x - p1y*p3z*p56x - p1z*p3x*p56y + 
                                p1x*p3z*p56y))*p5z) + 
                E1*(E5*(p2z*(-(p34z*p3y*p56x) + p34y*p3z*p56x + 
                                p34z*p3x*p56y - p34x*p3z*p56y) + 
                            (-(p2y*p34z*p3x) + p2x*p34z*p3y + p2y*p34x*p3z - 
                             p2x*p34y*p3z)*p56z) + 
                        (E56*p2z - E2*p56z)*
                        (p34z*p3y*p5x - p34y*p3z*p5x - p34z*p3x*p5y + 
                         p34x*p3z*p5y) + 
                        (E56*(p2y*p34z*p3x - p2x*p34z*p3y - p2y*p34x*p3z + 
                              p2x*p34y*p3z) + 
                         E2*(p34z*p3y*p56x - p34y*p3z*p56x - p34z*p3x*p56y + 
                             p34x*p3z*p56y))*p5z);

            inv_jac *= 8.*16.*SQ(M_PI*sqrt_s);

            return 1. / std::abs(inv_jac);
        }

    private:
        double sqrt_s;
        bool pT_is_met;

        // Inputs
        Value<double> s13;
        Value<double> s134;
        Value<double> s25;
        Value<double> s256;
        std::vector<Value<LorentzVector>> m_particles;
        Value<LorentzVector> m_met;

        // Outputs
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(BlockD);
