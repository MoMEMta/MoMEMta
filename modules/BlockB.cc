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


/** Final (main) Block B, consisting on \f$q_1 q_2 \to X + s_{12} \to X + p_1 p_2\f$,  
 * where \f$q_1\f$ and \f$q_2\f$ are Bjorken fractions, and \f$s_{12}\f$ is a particle decaying 
 * into \f$p_1\f$ (invisible particle) and \f$p_2\f$ (visible particle).
 *
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrization to the \f$\frac{1}{4\pi E_1} ds_{12} \times J\f$ parametrization.
 * 
 * The integration is performed over \f$s_{12}\f$ with \f$p_2\f$ as input. Per integration point, 
 * the LorentzVector of the invisible particle, \f$p_1\f$, is computed based on the following set 
 * of equations:   
 *
 * - \f$(p_1 + p_2)^2 = s_{12} = M_{1}^{2} + M_{2}^2 + 2 E_1 E_2 + 2 p_{1x}p_{2x} + 2p_{1y}p_{2y} + p_{1z}p_{2z}\f$
 * - Conservation of momentum (with \f$\vec{p}_T\f$ the total transverse momentum of visible particles):
 *  - \f$p_{1x} = - p_{Tx}\f$
 *  - \f$p_{1y} = - p_{Ty}\f$
 * - \f$M_1 = 0 \to E_{1}^2 = p_{1x}^2 + p_{1y}^2 + p_{1z}^2\f$
 *
 * Up to two \f$p_1\f$ solutions are possible.
 *
 * - Integration dimension: 0
 *
 * - Parameters:
 *  - `energy` (double): Collision energy.
 *
 * - Inputs:
 *  - `s12` (double): Invariant mass of the particle decaying into the missing particle (\f$p_1\f$) 
 *                    and the visible particle, \f$p_2\f$. Typically coming from a Flatter module.
 *  - `inputs` (vector(LorentzVector)): LorentzVector of all the experimentally reconstructed particles.
 *                                      In this Block there is only one visible particle, \f$p_2\f$.
 * - Outputs:
 *  - `invisibles` (vector(vector(LorentzVector))): LorentzVector of the invisible particles. In this Block \f$p_1\f$.
 *                                                  One value per solution.
 *  - `jacobians` (vector(double)): Jacobian of the performed change of variables, leading to an integration on \f$ds_{12}\f$
 *                                  One jacobian per solution.                                
 */

class BlockB: public Module {
    public:
  
        BlockB(PoolPtr pool, const ConfigurationSet& parameters): Module(pool, parameters.getModuleName()) {
            
            sqrt_s = parameters.globalConfiguration().get<double>("energy");
            
            s12 = get<double>(parameters.get<InputTag>("s12"));
            
            m_particle_tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& t: m_particle_tags)
              t.resolve(pool);
        }; 
  
        virtual void work() override {
      
            invisibles->clear();
            jacobians->clear();

            // Equations to solve:
            //(1) (p1 + p2)^2 = s12 = M1^2 + M2^2 + 2E1E2 + 2p1xp2x + 2p1yp2y + p1zp2z  
            //(2)  p1x = - pTx  #Coming from pT neutrino = -pT visible = - (p2 + ISR)
            //(3)  p1y = - pTy  #Coming from pT neutrino = -pT visible = - (p2 + ISR)
            //(4)  M1 = 0 -> E1^2 = p1x^2 + p1y^2 + p1z^2

            // Don't spend time on unphysical part of phase-space
            if(*s12 > SQ(sqrt_s))
                return;
            
            const LorentzVector& p2 = m_particle_tags[0].get<LorentzVector>();
            
            // FIXME
            const LorentzVector ISR; // = (*particles)[0];
            auto pT = p2 + ISR;
            
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
                return;

            for(unsigned int i=0; i<E1.size(); i++){
                const double e1 = E1.at(i);
                
                if (e1 < 0.) continue;
                
                LorentzVector p1(-pT.Px(), -pT.Py(), A - B*e1, e1);
                
                // Check if solutions are physical
                LorentzVector tot = p1 + p2;
                double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
                double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;
                if(q1Pz > sqrt_s/2 || q2Pz > sqrt_s/2)
                    continue;
                
                invisibles->push_back({p1});
                jacobians->push_back(computeJacobian(p1, p2));   
            }
        }

        // The extra dimensions not present in the input
        virtual size_t dimensions() const override {
            return 0;
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
  
        std::vector<InputTag> m_particle_tags;

        std::shared_ptr<const double> s12;

        std::shared_ptr<std::vector<std::vector<LorentzVector>>> invisibles = produce<std::vector<std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>>>>("invisibles");
        std::shared_ptr<std::vector<double>> jacobians = produce<std::vector<double>>("jacobians");
};
REGISTER_MODULE(BlockB);
