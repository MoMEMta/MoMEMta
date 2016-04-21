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

#include <momemta/ConfigurationSet.h>
#include <momemta/Module.h>
#include <momemta/Types.h>
#include <momemta/Utils.h>

#include <logging.h>

#include <TMath.h>

/*! \brief This module aims at computing the jacobians coming from the changes of variable in the
 *         standard phase-space parametrization for processes like WW->llMET.
 *
 * Final (main) Block F on \f$q_1 q_2 \to X + s_{13} + s_{24} \to X + p_1 p_2 p_3 p_4\f$,  
 * where \f$q_1\f$ and \f$q_2\f$ are Bjorken fractions, s_{13} and s_{24} are particles
 * decaying respectively into \f$p_1\f$ (invisible particle) and \f$p_3\f$ (visible particle),
 * and \f$p_2\f$ (invisible particle) and \f$p_4\f$ (visible particle).
 * 
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrization to the $\frac{1}{16\pi^2 E_1 E_2} dq_{1} dq_{2} ds_{13} d_s{24}  \times J$ parametrization.               .
 * 
 * The integration is performed over \f$q_{1}\f$, \f$q_{2}$f\, \f$s_{13}$f\ and \f$s_{24}\f$
 * with \f$p_3\f$ and \f$p_4\f$ as inputs. Per integration point, 
 * the LorentzVectors of the invisible particle, \f$p_1\f$ and \f$p_2\f$,
 * are computed  based on a set of equations.
 *
 * Up to 2 {\f$p_1\f$,\f$p_2$f\} solutions are possible.
 *
 * - Integration dimension: 2
 *
 * - Parameters:
 *  - `energy` (double): Collision energy.
 *
 *  - Inputs:
 *  - `q1` and `q2` (double): Bjorken fractions. These are the dimensions of integration coming from CUBA as phase-space points.
 *  - `s13` (double): Invariant mass of the particle decaying into the missing particle (\f$p_1\f$) 
 *                    and the visible particle (\f$p_3\f$). Typically coming from a Flatter module.
 *  - `s24` (double): Invariant mass of the particle decaying into the missing particle (\f$p_2\f$)
 *                    and the visible particle (\f$p_4\f$). Tipically coming from a Flatter module.
 *  - `inputs` (vector(LorentzVector)): LorentzVector of all the experimentally reconstructed particles.
 *                                      In this Block its dimension is 2.
 * - Outputs:
 *  - `invisibles` (vector(vector(LorentzVector))): LorentzVector of the invisible particles. Each element
 *                                                  contains one of the possible solutions ({\f$p1$f\,\f$p2$f\} in this case).
 *                                                 
 *  - `jacobians` (vector(double)): Jacobian of the performed change of variables, leading to an integration on \f$dq_1$f\,
 *                                  \f$dq_2$f\, \f$ds_{13}\f$, \f$ds_{24}$f\.
 *                                  One jacobian per solution.                                
 */

class BlockF: public Module {
    public:

  BlockF(PoolPtr pool, const ConfigurationSet& parameters): Module(pool, parameters.getModuleName()) {

            m_ps_point1 = parameters.get<InputTag>("q1");
            m_ps_point1.resolve(pool);

            m_ps_point2 = parameters.get<InputTag>("q2");
            m_ps_point2.resolve(pool);

            sqrt_s = parameters.globalConfiguration().get<double>("energy");

            s13 = get<double>(parameters.get<InputTag>("s13"));
            s24 = get<double>(parameters.get<InputTag>("s24"));
	    
            m_particle_tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& t: m_particle_tags)
                t.resolve(pool);
        };

        virtual void work() override {

            invisibles->clear();
            jacobians->clear();

            const LorentzVector& p3 = m_particle_tags[0].get<LorentzVector>();
            const LorentzVector& p4 = m_particle_tags[1].get<LorentzVector>();
           
            //leave the variables E2 and p2y as free parameters 
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
            
            double Eb = E3+E4;
            double pbx = p3x+p4x;
            double pby = p3y+p4y;
            double pbz = p3z+p4z;
            
            double q1 = m_ps_point1.get<double>();
            double q2 = m_ps_point2.get<double>();

            const double Qm = sqrt_s*(q1-q2)/2.;
            const double Qp = sqrt_s*(q1+q2)/2.;
            
            //p1x = alpha1*p2y + beta1*E2 + gamma1
            //p1y = alpha2*p2y + beta2*E2 + gamma2
            //p1z = alpha3*p2y + beta3*E2 + gamma3
            //p2x = alpha4*p2y + beta4*E2 + gamma4
            //p2z = alpha5*p2y + beta5*E2 + gamma5
            //E1 = alpha6*p2y + beta6*E2 + gamma6
             
            double alpha1 = (-p3z*p4y+p3y*p4z)/(-p3z*p4x+p3x*p4z);
            double beta1 = (E4*p3z-E3*p4z)/(p3z*p4x+p3x*p4z);
            double gamma1 = (p44*p3z-2*E3*Eb*p4z+p33*p4z+2*p3z*p4x*pbx+
                            2*p3y*p4z*pby+2*p3z*p4z*pbz-2*p3z*p4z*Qm+2*E3*p4z*Qp-
                            p4z*(*s13)-p3z*(*s24))/(2*(-p3z*p4x+p3x*p4z));
            
            double alpha2 = -1;
            double beta2 = 0;
            double gamma2 = -pby;
            
            double alpha3 = (p3y*p4x-p3x*p4y)/(p3z*p4x-p3x*p4z);
            double beta3 = (E4*p3x-E3*p4x)/(p3z*p4x-p3x*p4z);
            double gamma3 = 0.5*((p44*p3x-2*E3*Eb*p4x+p33*p4x+2*p3x*p4x*pbx+
                            2*p3y*p4x*pby+2*p3x*p4z*pbz-2*p3x*p4z*Qm+2*E3*p4x*Qp-
                            p4x*(*s13)-p3x*(*s24))/(p3z*p4x-p3x*p4z));
            
            double alpha4 = (p3z*p4y-p3y*p4z)/(-p3z*p4x+p3x*p4z);
            double beta4 = (-E4*p3z+E3*p4z)/(-p3z*p4x+p3x*p4z);
            double gamma4 = (-2*p44*p3z+4*E3*Eb*p4z-2*p33*p4z-
                             4*p3z*p4x*pbx-4*p3y*p4z*pby-4*p3z*p4z*pbz+
                             4*p3z*p4z*Qm-4*E3*p4z*Qp+2*p4z*(*s13)+
                             2*p3z*(*s24))/(-4*p3z*p4x+4*p3x*p4z)-pbx;
              
            double alpha5 = (-p3y*p4x+p3x*p4y)/(p3z*p4x-p3x*p4z);
            double beta5 = (-E4*p3x+E3*p4x)/(p3z*p4x-p3x*p4z);
            double gamma5 = 0.5*((-p44*p3x+2*E3*Eb*p4x-p33*p4x-
                            2*p3x*p4x*pbx-2*p3y*p4x*pby-2*p3z*p4x*pbz+
                            2*p3z*p4x*Qm-2*E3*p4x*Qp+p4x*(*s13)+
                            p3x*(*s24))/(p3z*p4x-p3x*p4z));
            
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
            
            double b11 = 1 + SQ(alpha4) + SQ(alpha5);
            double b22 = SQ(beta4) + SQ(beta5) -1;
            double b12 = 2*(alpha4*beta4 + alpha5*beta5);
            double b10 = 2*(alpha4*gamma4 + alpha5*gamma5);
            double b01 = 2*(beta4*gamma4 + beta5*gamma5);
            double b00 = SQ(gamma4) + SQ(gamma5);
            
            solve2Quads(a11, a22, a12, a10, a01, a00, b11, b22, b12, b10, b01, b00, p2y, E2, false);
            //In this case there will be up to 2 solutions
            
            if (p2y.size() == 0)
                return;
            
            for (size_t i=0; i<p2y.size(); i++) {
                const double e1 = p2y.at(i);
                const double e2 = E2.at(i);
                
                if (e1 < 0. || e2 < 0.)
                    continue;
                
                LorentzVector p1(alpha6*e1+beta6*e2+gamma6,     //E1
                                 alpha1*e1+beta1*e2+gamma1,     //p1x
                                 alpha2*e1+beta2*e2+gamma2,     //p1y
                                 alpha3*e1+beta3*e2+gamma3      //p1z
                                 );
                
                LorentzVector p2(e2,                            //E2
                                 alpha4*e1+beta4*e2+gamma4,     //p2x
                                 e1,                            //p2y
                                 alpha5*e1+beta5*e2+gamma5      //p2z
                                 );                  
                
                invisibles->push_back({p1, p2});
                jacobians->push_back(computeJacobian(p1, p2, p3, p4));
            }    
        }
    
        virtual size_t dimensions() const override {
            return 2;
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

        std::vector<InputTag> m_particle_tags;

        std::shared_ptr<const double> s13;
        std::shared_ptr<const double> s24;
        InputTag m_ps_point1;
        InputTag m_ps_point2;

        std::shared_ptr<std::vector<std::vector<LorentzVector>>> invisibles = produce<std::vector<std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>>>>("invisibles");
        std::shared_ptr<std::vector<double>> jacobians = produce<std::vector<double>>("jacobians");
};
REGISTER_MODULE(BlockF);
