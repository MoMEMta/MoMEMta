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

#include <functional>

#include <Math/Vector3D.h>
#include <Math/Boost.h>

#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Types.h>

/** \brief Build the initial partons given the whole final state
 *
 * Define the initial partons 4-momenta given the visible and invisible particles present in the final state.
 *
 * If no invisibles are present, only one pair of initial partons is output. If there are invisibles, there can
 * be several entries of them (since blocks typically output several solutions): in that case, one pair of initial partons per solution is defined.
 *
 * If the parameter `do_transverse_boost` is set to `true`, the partons are boosted in the transverse direction
 * to satisfy energy-momentum conservation. 
 * Otherwise, the partons are simply built using the total energy and longitudinal momentum, without enforcing
 * conservation of transverse momentum.
 *
 * Use of the transverse boost or not can depend on how the invisibles are reconstructed by the blocks: 
 * if the block already enforces momentum conservation, applying the boost or not will yield the same result.
 *
 * ### Integration dimension
 *
 * This module adds **0** dimension to the integration.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `do_transverse_boost` | bool, default: `false` | Boost the initial partons in the transverse direction to match total transverse momentum. | 
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `particles` | vector(LorentzVector) | Set of all visible particles. | 
 *   | `invisibles` | vector(vector(LorentzVector)) | Solutions for invisible particles coming out of a block (typically, several solutions are present). | 
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `output` | vector(vector(LorentzVector)) | Sets of initial parton 4-momenta (one per invisibles' solution). | 
 *
 * \ingroup modules
 */
class BuildInitialState: public Module {
    public:

        BuildInitialState(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            if (parameters.get<bool>("do_transverse_boost", false)) {
                do_compute_initials = compute_initials_boost;
            } else {
                do_compute_initials = compute_initials_trivial;
            }
            
            std::vector<InputTag> invisibles_tag = parameters.get<std::vector<InputTag>>("invisibles");
            for (const auto& tag: invisibles_tag) {
                invisibles.push_back(get<std::vector<std::vector<LorentzVector>>>(tag));
            }

            input_particles = parameters.get<std::vector<InputTag>>("particles");
            for (auto& t: input_particles)
                t.resolve(pool);
        };

        virtual void work() override {

            output->clear();

            std::vector<LorentzVector> particles;
            for (auto& p: input_particles) {
                particles.push_back(p.get<LorentzVector>());
            }

            if (!invisibles.empty()) {

                size_t n_sols = invisibles.front()->size();
                for (const auto& i: invisibles) {
                    if (i->size() != n_sols) {
                        throw std::runtime_error("All invisibles must have the same number of solutions.");
                    }
                }

                for (size_t sol_index = 0; sol_index < n_sols; sol_index++) {

                    std::vector<LorentzVector> particles_and_invisibles = particles;

                    for (const auto& i: invisibles) {

                        for (const auto& p: (*i)[sol_index])
                            particles_and_invisibles.push_back(p);
                    
                    }
                
                    do_compute_initials(particles_and_invisibles);
                }
                
            } else {
                do_compute_initials(particles);
            }
        }

    private:
        std::function<void(const std::vector<LorentzVector>&)> do_compute_initials; 

        std::function<void(const std::vector<LorentzVector>&)> compute_initials_trivial = 
            [&](const std::vector<LorentzVector>& particles) {
                LorentzVector tot;
                for (const auto& p: particles)
                    tot += p;

                double q1Pz = (tot.Pz() + tot.E()) / 2.;
                double q2Pz = (tot.Pz() - tot.E()) / 2.;
                
                std::vector<LorentzVector> partons {
                    LorentzVector(0., 0., q1Pz, std::abs(q1Pz)),
                    LorentzVector(0., 0., q2Pz, std::abs(q2Pz))
                };

                output->push_back(partons);
            };

        std::function<void(const std::vector<LorentzVector>&)> compute_initials_boost = 
            [&](const std::vector<LorentzVector>& particles) {
                LorentzVector tot;
                for (const auto& p: particles)
                    tot += p;
                
                // Define boost that puts the transverse total momentum vector in its CoM frame 
                LorentzVector transverse_tot = tot;
                transverse_tot.SetPz(0);
                ROOT::Math::XYZVector isr_deBoost_vector( transverse_tot.BoostToCM() );

                // In the "transverse" CoM frame, use total Pz and E to define initial longitudinal quark momenta
                const ROOT::Math::Boost isr_deBoost( isr_deBoost_vector );
                const ROOT::Math::PxPyPzEVector new_tot( isr_deBoost * tot );

                double q1Pz = (new_tot.Pz() + new_tot.E()) / 2.;
                double q2Pz = (new_tot.Pz() - new_tot.E()) / 2.;

                std::vector<LorentzVector> partons {
                    LorentzVector(0., 0., q1Pz, std::abs(q1Pz)),
                    LorentzVector(0., 0., q2Pz, std::abs(q2Pz))
                };

                // Boost initial parton momenta by the opposite of the transverse boost needed to put the whole system in its CoM
                const ROOT::Math::Boost isr_boost( -isr_deBoost_vector );
                partons[0] = isr_boost*partons[0];
                partons[1] = isr_boost*partons[1];

                output->push_back(partons);
            };

        std::vector<std::shared_ptr<const std::vector<std::vector<LorentzVector>>>> invisibles;
        std::vector<InputTag> input_particles;

        std::shared_ptr<std::vector<std::vector<LorentzVector>>> output = produce<std::vector<std::vector<LorentzVector>>>("output");
};
REGISTER_MODULE(BuildInitialState);
