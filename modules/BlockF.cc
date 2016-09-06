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
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Solution.h>
#include <momemta/Types.h>
#include <momemta/Math.h>

#include <TMath.h>

/*! \brief \f$\require{cancel}\f$ Final (main) Block F, describing \f$q_1 q_2 \to X + s_{13} (\to \cancel{p_1} p_3) + s_{24} (\to \cancel{p_2} p_4)\f$
 *
 * Final (main) Block F on \f$q_1 q_2 \to X + s_{13} + s_{24} \to X + p_1 p_2 p_3 p_4\f$,  
 * where \f$q_1\f$ and \f$q_2\f$ are Bjorken fractions, \f$s_{13}\f$ and \f$s_{24}\f$ are particles
 * decaying respectively into \f$p_1\f$ (invisible particle) and \f$p_3\f$ (visible particle),
 * and \f$p_2\f$ (invisible particle) and \f$p_4\f$ (visible particle).
 * 
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrization to the \f$\frac{1}{16\pi^2 E_1 E_2} dq_{1} dq_{2} ds_{13} d_s{24}  \times J\f$ parametrization.
 * 
 * The integration is performed over \f$q_{1}\f$, \f$q_{2}\f$, \f$s_{13}\f$ and \f$s_{24}\f$
 * with \f$p_3\f$ and \f$p_4\f$ as inputs. Per integration point, 
 * the LorentzVectors of the invisible particle, \f$p_1\f$ and \f$p_2\f$,
 * are computed  based on this set of equations:
 *
 * - \f$s_{13} = (p_1 + p_3)^2\f$
 * - \f$s_{24} = (p_2 + p_4)^2\f$
 * - Conservation of momentum (with \f$p^{tot}\f$ the total momentum of visible particles):
 *  - \f$p_{1x} + p_{2x} = - p_{x}^{tot}\f$
 *  - \f$p_{1y} + p_{2y} = - p_{y}^{tot}\f$
 * - \f$p_{1z} + p_{2z} = s^{1/2} (q_1 - q_2)/2 - E^{tot}\f$
 * - \f$E_{1} + E_{2} = s^{1/2} (q_1 + q_2)/2 - E^{tot}\f$
 *
 * The observed MET is not used in this block since to reconstruct the
 * neutrinos the system requires as input the total 4-momentum of the 
 * visible objects (energy and longitudinal momentum included).
 *
 * Up to 2 (\f$p_1\f$, \f$p_2\f$) solutions are possible.
 *
 *
 * ### Integration dimension
 *
 * This module requires **2** phase-space points.
 *
 * ### Global parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|-------------|
 *   | `energy` | double | Collision energy |
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|-------------|
 *   | `q1` <br /> `q2` | double | Bjorken fractions. These are the dimensions of integration coming from CUBA as phase-space points |
 *   | `s13` | double | Invariant mass of the particle decaying into the missing particle (\f$p_1\f$) and the visible particle (\f$p_3\f$). Typically coming from a BreitWignerGenerator module. |
 *   | `s24` | double | Invariant mass of the particle decaying into the missing particle (\f$p_2\f$) and the visible particle (\f$p_4\f$). Typically coming from a BreitWignerGenerator module. |
 *   | `inputs` | vector of LorentzVector | LorentzVectors of all the experimentally reconstructed particles. The first two entries correspond to \f$p_3\f$ and \f$p_4\f$, the others (if present) are used to compute \f$p^{tot}\f$. |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|-------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embed  the LorentzVectors of the invisible particles (\f$p1\f$, \f$p2\f$ in this case) and the associated jacobian. These solutions should be fed as input to the Looper module. |
 *
 * \note This block has been validated and is safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */

class BlockF: public Module {
    public:

  BlockF(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {

            m_ps_point1 = get<double>(parameters.get<InputTag>("q1"));
            m_ps_point2 = get<double>(parameters.get<InputTag>("q2"));

            sqrt_s = parameters.globalParameters().get<double>("energy");

            s13 = get<double>(parameters.get<InputTag>("s13"));
            s24 = get<double>(parameters.get<InputTag>("s24"));
	    
            auto particle_tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& t: particle_tags)
                m_particles.push_back(get<LorentzVector>(t));
        };

        virtual Status work() override {

            solutions->clear();

            // Don't spend time on unphysical part of phase-space
            if (*s13 > SQ(sqrt_s) || *s24 > SQ(sqrt_s))
                return Status::NEXT;
            
            const LorentzVector& p3 = *m_particles[0];
            const LorentzVector& p4 = *m_particles[1];
           
            // Leave the variables E2 and p2y as free parameters 
            std::vector<double> E2;
            std::vector<double> p2y;
            
            double p3x = p3.Px();
            double p3y = p3.Py();
            double p3z = p3.Pz();
            double E3 = p3.E();
            
            double p4x = p4.Px();
            double p4y = p4.Py();
            double p4z = p4.pz();
            double E4 = p4.E();
            
            double p33 = p3.M2();
            double p44 = p4.M2();
            
            // Total visible momentum
            LorentzVector pb = p3 + p4;
            for (size_t i = 2; i < m_particles.size(); i++) {
                pb += *m_particles[i];
            }
            double Eb = pb.E();
            double pbx = pb.Px();
            double pby = pb.Py();
            double pbz = pb.Pz();
            
            double q1 = *m_ps_point1;
            double q2 = *m_ps_point2;

            const double Qm = sqrt_s*(q1-q2)/2.;
            const double Qp = sqrt_s*(q1+q2)/2.;
            
            // p1x = alpha1*p2y + beta1*E2 + gamma1
            // p1y = alpha2*p2y + beta2*E2 + gamma2
            // p1z = alpha3*p2y + beta3*E2 + gamma3
            // p2x = alpha4*p2y + beta4*E2 + gamma4
            // p2z = alpha5*p2y + beta5*E2 + gamma5
            // E1 = alpha6*p2y + beta6*E2 + gamma6
            
            double den = p3z*p4x-p3x*p4z;

            double alpha1 = (p3z*p4y-p3y*p4z)/den;
            double beta1 = (-E4*p3z+E3*p4z)/den;
            double gamma1 = -(p44*p3z-2*E3*Eb*p4z+p33*p4z+2*p3z*p4x*pbx+
                            2*p3y*p4z*pby+2*p3z*p4z*pbz-2*p3z*p4z*Qm+2*E3*p4z*Qp-
                            p4z*(*s13)-p3z*(*s24))/(2*den);
            
            double alpha2 = -1;
            double beta2 = 0;
            double gamma2 = -pby;
            
            double alpha3 = (p3y*p4x-p3x*p4y)/den;
            double beta3 = (E4*p3x-E3*p4x)/den;
            double gamma3 = (p44*p3x-2*E3*Eb*p4x+p33*p4x+2*p3x*p4x*pbx+
                            2*p3y*p4x*pby+2*p3x*p4z*pbz-2*p3x*p4z*Qm+2*E3*p4x*Qp-
                             p4x*(*s13)-p3x*(*s24))/(2*den);
            
            double alpha4 = -alpha1;
            double beta4 = (E4*p3z-E3*p4z)/den;
            double gamma4 = -(-2*p44*p3z+4*E3*Eb*p4z-2*p33*p4z-
                             4*p3x*p4z*pbx-4*p3y*p4z*pby-4*p3z*p4z*pbz+
                             4*p3z*p4z*Qm-4*E3*p4z*Qp+2*p4z*(*s13)+
                              2*p3z*(*s24))/(4*den);
              
            double alpha5 = -alpha3;
            double beta5 = (-E4*p3x+E3*p4x)/den;
            double gamma5 = (-p44*p3x+2*E3*Eb*p4x-p33*p4x-
                            2*p3x*p4x*pbx-2*p3y*p4x*pby-2*p3z*p4x*pbz+
                            2*p3z*p4x*Qm-2*E3*p4x*Qp+p4x*(*s13)+
                             p3x*(*s24))/(2*den);
            
            double alpha6 = 0;
            double beta6 = -1;
            double gamma6 = -Eb+Qp;

            //Put these variables in p1^2=0, p2^2=0. You will get:
            //a11*p2y^2 + a22*E2^2 + a12*p2y*E2 + a10*p2y + a01*E2 + a00 = 0;
            //same with bij
               
            double a11 = SQ(alpha6) - SQ(alpha1) - SQ(alpha2) - SQ(alpha3);
            double a22 = SQ(beta6) - SQ(beta1) - SQ(beta2) - SQ(beta3);
            double a12 = 2*(alpha6*beta6 - alpha1*beta1 - alpha2*beta2 - alpha3*beta3);
            double a10 = 2*(alpha6*gamma6 - alpha1*gamma1 - alpha2*gamma2 - alpha3*gamma3);
            double a01 = 2*(beta6*gamma6 - beta1*gamma1 - beta2*gamma2 - beta3*gamma3);
            double a00 = SQ(gamma6) - SQ(gamma1) - SQ(gamma2) - SQ(gamma3);
 
            double b10 = 2*(alpha4*gamma4 + alpha5*gamma5);
            double b01 = 2*(beta4*gamma4 + beta5*gamma5);
            double b00 = SQ(gamma4) + SQ(gamma5);

            //These 2 equations are equivalent to the following system:
            //a11*p2y^2 + a22*E2^2 + a12*p2y*E2 + a10*p2y + a01*E2 + a00 = 0
            //E2 = c0 + c1*p2y
            //From these two, we get a quadratic equation in p2y:
            //d2*p2y^2 + d1*p2y + d0 = 0

            double c0 = -(a00+b00)/(a01+b01);
            double c1 = -(a10+b10)/(a01+b01);

            double d0 = a22*SQ(c0) + a01*c0 + a00;
            double d1 = 2*a22*c1*c0 + a12*c0 + a01*c1 + a10;
            double d2 = a22*SQ(c1) + a12*c1 + a11;

            solveQuadratic(d2, d1, d0, p2y, false);

            if (p2y.size() == 0)
                return Status::NEXT;
            
            for (size_t i=0; i<p2y.size(); i++) {
                const double e1 = p2y.at(i);      //p2y
                const double e2 = (c0 + c1*e1);   //E2
                
                if (e2 < 0.)
                    continue;
                
                LorentzVector p1(alpha1*e1+beta1*e2+gamma1,     //p1x
                                 alpha2*e1+beta2*e2+gamma2,     //p1y
                                 alpha3*e1+beta3*e2+gamma3,     //p1z
                                 alpha6*e1+beta6*e2+gamma6      //E1
                                 );

                if (p1.E() < 0.)
                    continue;

                LorentzVector p2(alpha4*e1+beta4*e2+gamma4,     //p2x
                                 e1,                            //p2y
                                 alpha5*e1+beta5*e2+gamma5,     //p2z
                                 e2                             //E2
                                 );                  

                // Check if solutions are physical
                LorentzVector tot = p1 + p2;
                for (size_t i = 0; i < m_particles.size(); i++){
                    tot += *m_particles[i];
                }
                double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
                double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;
                
                if(q1Pz > sqrt_s/2 || q2Pz > sqrt_s/2)
                    continue;
                
                auto jacobian = computeJacobian(p1, p2, p3, p4);
                Solution s { {p1, p2}, jacobian, true };
                solutions->push_back(s);
            }

            return solutions->size() > 0 ? Status::OK : Status::NEXT;
        }
    
        double computeJacobian(const LorentzVector& p1, const LorentzVector& p2, const LorentzVector& p3, const LorentzVector& p4) {

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

            double inv_jac= (E4*(p1z*p2y*p3x - p1y*p2z*p3x - p1z*p2x*p3y + p1x*p2z*p3y + p1y*p2x*p3z - p1x*p2y*p3z) +  E2*p1z*p3y*p4x - E1*p2z*p3y*p4x - E2*p1y*p3z*p4x + E1*p2y*p3z*p4x - E2*p1z*p3x*p4y + E1*p2z*p3x*p4y +  E2*p1x*p3z*p4y - E1*p2x*p3z*p4y + (E2*p1y*p3x - E1*p2y*p3x - E2*p1x*p3y + E1*p2x*p3y)*p4z + E3*(-(p1z*p2y*p4x) + p1y*p2z*p4x + p1z*p2x*p4y - p1x*p2z*p4y - p1y*p2x*p4z + p1x*p2y*p4z));
            
            return 1./( std::abs(inv_jac) * 4.*16.*pow(TMath::Pi(),2.) );
        }

    private:
        double sqrt_s;

        // Inputs
        Value<double> s13;
        Value<double> s24;
        Value<double> m_ps_point1;
        Value<double> m_ps_point2;
        std::vector<Value<LorentzVector>> m_particles;

        // Outputs
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(BlockF);
