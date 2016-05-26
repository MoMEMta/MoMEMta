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
#include <momemta/Types.h>
#include <momemta/Utils.h>


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
 * This module adds **0** dimension to the integration.
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
 *   | `invisibles` | vector(vector(LorentzVector)) | LorentzVector of the invisible particles. In this Block \f$p_1\f$. One value per solution.
 *   | `jacobians` | vector(double) | Jacobian of the performed change of variables, leading to an integration on \f$ds_{12}\f$. One jacobian per solution.
 *
 * \warning This block is **not** validated for the moment. The output is maybe correct, maybe not. Use with caution.
 *
 * \ingroup modules
 */

class BlockB: public Module {
    public:
  
        BlockB(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            
            sqrt_s = parameters.globalParameters().get<double>("energy");
            pT_is_met = parameters.get<bool>("pT_is_met", false);
            
            s12 = get<double>(parameters.get<InputTag>("s12"));
            
            m_particle_tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& t: m_particle_tags)
              t.resolve(pool);
            
            // If the met input is specified, get it, otherwise retrieve default
            // one ("input::met")
            if (parameters.exists("met")) {
                m_met_tag = parameters.get<InputTag>("met");
            } else {
                m_met_tag = InputTag({"input", "met"});
            }
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
            
            LorentzVector pT;
            if (pT_is_met) {
                pT = - m_met_tag.get<LorentzVector>(); 
            } else {
                pT = p2;
                for (size_t i = 1; i < m_particle_tags.size(); i++) {
                    pT += m_particle_tags[i].get<LorentzVector>();
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
                return;

            for(unsigned int i=0; i<E1.size(); i++){
                const double e1 = E1.at(i);
                
                if (e1 < 0.) continue;
                
                LorentzVector p1(-pT.Px(), -pT.Py(), A - B*e1, e1);
                
                // Check if solutions are physical
                LorentzVector tot = p1;
                for (size_t i = 0; i < m_particle_tags.size(); i++) {
                    tot += m_particle_tags[i].get<LorentzVector>();
                }
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
        bool pT_is_met;
  
        std::vector<InputTag> m_particle_tags;
        InputTag m_met_tag;

        std::shared_ptr<const double> s12;

        std::shared_ptr<std::vector<std::vector<LorentzVector>>> invisibles = produce<std::vector<std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>>>>("invisibles");
        std::shared_ptr<std::vector<double>> jacobians = produce<std::vector<double>>("jacobians");
};
REGISTER_MODULE(BlockB);
